#include "WavetableOscillator.h"

namespace usam::dsp
{

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------
WavetableOscillator::WavetableOscillator()
{
    // Forces the shared tables to be built on the constructing (non-audio)
    // thread, and guarantees `table` is never null.
    updateTablePointer();
}

void WavetableOscillator::prepare (double newSampleRate)
{
    sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
    reset();

    // Re-derive the mip level: the same frequency allows a different number
    // of harmonics at a different sample rate.
    if (frequency > 0.0f)
        setFrequency (frequency);
}

void WavetableOscillator::setWaveform (Waveform newWaveform) noexcept
{
    if (newWaveform < Waveform::sine || newWaveform >= Waveform::count)
        newWaveform = Waveform::sine;

    if (waveform != newWaveform)
    {
        waveform = newWaveform;
        updateTablePointer();
    }
}

// ---------------------------------------------------------------------------
// Parameter setters (audio-thread safe: value writes and pointer swaps only)
// ---------------------------------------------------------------------------
void WavetableOscillator::setFrequency (float frequencyHz) noexcept
{
    frequency = frequencyHz;
    phaseIncrement = static_cast<double> (frequencyHz) * static_cast<double> (tableSize)
                     / sampleRate;

    const int newLevel = mipLevelForFrequency (frequencyHz);
    if (newLevel != mipLevel)
    {
        mipLevel = newLevel;
        updateTablePointer();
    }
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

void WavetableOscillator::reset() noexcept
{
    phase = 0.0;
}

void WavetableOscillator::setPhase (float normalisedPhase) noexcept
{
    normalisedPhase -= std::floor (normalisedPhase); // wrap into 0..1
    phase = static_cast<double> (normalisedPhase) * static_cast<double> (tableSize);
}

int WavetableOscillator::mipLevelForFrequency (float frequencyHz) const noexcept
{
    if (frequencyHz <= 0.0f)
        return numMipLevels - 1; // no meaningful pitch: use the fullest table

    const float allowedHarmonics = static_cast<float> (0.5 * sampleRate) / frequencyHz;
    if (allowedHarmonics <= 1.0f)
        return 0; // fundamental only

    return juce::jlimit (0, numMipLevels - 1,
                         static_cast<int> (std::log2 (allowedHarmonics)));
}

void WavetableOscillator::updateTablePointer() noexcept
{
    table = &getSharedTables().tables[static_cast<size_t> (waveform)]
                                     [static_cast<size_t> (mipLevel)];
}

// ---------------------------------------------------------------------------
// Table generation (once, on first construction — never on the audio thread
// afterwards; the tables are immutable from then on)
// ---------------------------------------------------------------------------
namespace
{
    // Fourier-series amplitude of harmonic n for each waveform (0 when the
    // waveform does not contain that harmonic). Overall scale is irrelevant —
    // every table is peak-normalized after summation.
    double harmonicAmplitude (WavetableOscillator::Waveform w, int n)
    {
        using Waveform = WavetableOscillator::Waveform;

        switch (w)
        {
            case Waveform::sine:
                return n == 1 ? 1.0 : 0.0;

            case Waveform::saw: // all harmonics, 1/n
                return 1.0 / static_cast<double> (n);

            case Waveform::square: // odd harmonics, 1/n
                return n % 2 == 1 ? 1.0 / static_cast<double> (n) : 0.0;

            case Waveform::triangle: // odd harmonics, 1/n^2, alternating sign
                if (n % 2 == 0)
                    return 0.0;
                return (((n - 1) / 2) % 2 == 0 ? 1.0 : -1.0)
                       / static_cast<double> (n) / static_cast<double> (n);

            default:
                return 0.0;
        }
    }
} // namespace

WavetableOscillator::SharedTables::SharedTables()
{
    constexpr double twoPi = 6.28318530717958647692;

    for (size_t w = 0; w < tables.size(); ++w)
    {
        for (int level = 0; level < numMipLevels; ++level)
        {
            const int maxHarmonic = 1 << level;
            std::array<double, tableSize> acc{};

            for (int n = 1; n <= maxHarmonic; ++n)
            {
                const double amplitude = harmonicAmplitude (static_cast<Waveform> (w), n);
                if (amplitude == 0.0)
                    continue;

                for (int i = 0; i < tableSize; ++i)
                    acc[static_cast<size_t> (i)] +=
                        amplitude * std::sin (twoPi * n * i / static_cast<double> (tableSize));
            }

            // Peak-normalize so every (waveform, level) table spans ±1. This
            // also absorbs the Fourier scale factors and keeps loudness
            // consistent when the mip level changes with pitch.
            double peak = 0.0;
            for (const double v : acc)
                peak = std::max (peak, std::abs (v));
            const double scale = peak > 0.0 ? 1.0 / peak : 1.0;

            auto& t = tables[w][static_cast<size_t> (level)];
            for (int i = 0; i < tableSize; ++i)
                t[static_cast<size_t> (i)] =
                    static_cast<float> (acc[static_cast<size_t> (i)] * scale);
        }
    }
}

const WavetableOscillator::SharedTables& WavetableOscillator::getSharedTables()
{
    static const SharedTables sharedTables;
    return sharedTables;
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

    return (*table)[static_cast<size_t> (index0)] * (1.0f - frac)
           + (*table)[static_cast<size_t> (index1)] * frac;
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
