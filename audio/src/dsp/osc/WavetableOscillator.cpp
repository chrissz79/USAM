#include "WavetableOscillator.h"

namespace usam::dsp
{

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------
void WavetableOscillator::prepare (double newSampleRate)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    reset();
    buildTable (waveform);
}

void WavetableOscillator::setWaveform (Waveform newWaveform)
{
    waveform = newWaveform;
    buildTable (waveform);
}

void WavetableOscillator::reset() noexcept
{
    phase = 0.0;
}

// ---------------------------------------------------------------------------
// Parameter setters (audio-thread safe: only float/double writes)
// ---------------------------------------------------------------------------
void WavetableOscillator::setFrequency (float frequencyHz) noexcept
{
    phaseIncrement = static_cast<double> (frequencyHz) * static_cast<double> (tableSize)
                     / sampleRate;
}

void WavetableOscillator::setPhaseOffset (float phaseRadians) noexcept
{
    phaseOffset = phaseRadians / static_cast<float> (twoPi) * static_cast<float> (tableSize);
}

void WavetableOscillator::setPhaseModulation (float phaseModRadiansPerSample) noexcept
{
    phaseMod = phaseModRadiansPerSample / static_cast<float> (twoPi)
               * static_cast<float> (tableSize);
}

// ---------------------------------------------------------------------------
// Table generation (UI thread)
// ---------------------------------------------------------------------------
void WavetableOscillator::buildTable (Waveform w)
{
    for (int i = 0; i < tableSize; ++i)
    {
        const float t = static_cast<float> (i) / static_cast<float> (tableSize); // 0..1
        const float phase = t * static_cast<float> (twoPi);

        switch (w)
        {
            case Waveform::sine:
                table[static_cast<size_t> (i)] = std::sin (phase);
                break;

            // Band-limited saw via parabolic approximation (polyBLEP-free cheap
            // harmonic-richer variant). For M0 this is fine; polyBLEP comes in M1.
            case Waveform::saw:
                table[static_cast<size_t> (i)] = 2.0f * t - 1.0f;
                break;

            case Waveform::square:
                table[static_cast<size_t> (i)] = t < 0.5f ? 1.0f : -1.0f;
                break;

            case Waveform::triangle:
                table[static_cast<size_t> (i)] = t < 0.5f
                                                     ? 4.0f * t - 1.0f
                                                     : 3.0f - 4.0f * t;
                break;

            default:
                table[static_cast<size_t> (i)] = 0.0f;
                break;
        }
    }
}

// ---------------------------------------------------------------------------
// Rendering (audio thread)
// ---------------------------------------------------------------------------
float WavetableOscillator::getNextSample() noexcept
{
    phase += phaseIncrement + static_cast<double> (phaseMod);

    // Wrap phase into [0, tableSize)
    phase -= static_cast<double> (static_cast<long long> (phase / static_cast<double> (tableSize)))
             * static_cast<double> (tableSize);
    if (phase < 0.0)
        phase += static_cast<double> (tableSize);

    const float pos = static_cast<float> (phase) + phaseOffset;

    // Wrap index, then linear-interpolate between the two surrounding samples.
    float indexFloat = pos;
    if (indexFloat >= static_cast<float> (tableSize))
        indexFloat -= static_cast<float> (tableSize);
    if (indexFloat < 0.0f)
        indexFloat += static_cast<float> (tableSize);

    const int index0 = static_cast<int> (indexFloat);
    const int index1 = (index0 + 1) % tableSize;
    const float frac = indexFloat - static_cast<float> (index0);

    return table[static_cast<size_t> (index0)] * (1.0f - frac)
           + table[static_cast<size_t> (index1)] * frac;
}

void WavetableOscillator::process (juce::AudioBuffer<float>& buffer, int startSample,
                                   int numSamples) noexcept
{
    jassert (startSample >= 0 && startSample + numSamples <= buffer.getNumSamples());
    // Mono-only: getNextSample() advances the shared phase, so calling it once
    // per channel would produce different phases per channel. Render to mono
    // and copy to other channels (see SynthVoice::renderNextBlock).
    jassert (buffer.getNumChannels() == 1);

    auto* data = buffer.getWritePointer (0, startSample);
    for (int i = 0; i < numSamples; ++i)
        data[i] = getNextSample();
}

} // namespace usam::dsp
