#include "SynthVoice.h"

namespace usam
{

// Semitone offset (relative to A4 = 440 Hz at semitone 69) -> frequency in Hz.
// Takes a float so coarse/fine detune compose as fractional semitones.
static float semitonesToFrequency (float semitones) noexcept
{
    return 440.0f * std::pow (2.0f, (semitones - 69.0f) / 12.0f);
}

// Unison voice u of n gets a symmetric position in [-1, 1] (0 when n == 1).
static float unisonPosition (int u, int n) noexcept
{
    if (n <= 1)
        return 0.0f;
    return 2.0f * static_cast<float> (u) / static_cast<float> (n - 1) - 1.0f;
}

void SynthVoice::prepare (double newSampleRate, int maxBlockSize)
{
    sampleRate = newSampleRate;

    // The scratch buffer must be able to hold the largest host block.
    scratch.setSize (2, juce::jmax (1, maxBlockSize));

    for (auto& osc : osc1Bank)
        osc.prepare (sampleRate);
    for (auto& osc : osc2Bank)
        osc.prepare (sampleRate);
    subOsc.prepare (sampleRate);

    filter.prepare (sampleRate);
    ampEnv.prepare (sampleRate);
    reset();
}

void SynthVoice::reset() noexcept
{
    for (auto& osc : osc1Bank)
        osc.reset();
    for (auto& osc : osc2Bank)
        osc.reset();
    subOsc.reset();
    filter.reset();
    ampEnv.reset();
    scratch.clear();
    currentNote = -1;
    velocity = 0.0f;
}

void SynthVoice::updateBankTuning (OscBank& bank, const OscillatorParameters& oscParams) noexcept
{
    const int unison = juce::jlimit (1, maxUnisonVoices, oscParams.unisonVoices);
    const float baseSemitones = static_cast<float> (currentNote)
                                + oscParams.coarseSemitones
                                + oscParams.detuneCents / 100.0f;

    for (int u = 0; u < unison; ++u)
    {
        const float detuneCents = unisonPosition (u, unison) * oscParams.unisonDetuneCents;
        bank[static_cast<size_t> (u)].setWaveform (oscParams.waveform);
        bank[static_cast<size_t> (u)].setFrequency (
            semitonesToFrequency (baseSemitones + detuneCents / 100.0f));
    }
}

void SynthVoice::startNote (int midiNote, float newVelocity, const SynthParameters& params) noexcept
{
    currentNote = midiNote;
    velocity = juce::jlimit (0.0f, 1.0f, newVelocity);

    updateBankTuning (osc1Bank, params.osc1);
    updateBankTuning (osc2Bank, params.osc2);

    // Fan unison phases out across the cycle so the stack doesn't start as a
    // phase-coherent (comb-prone) burst; voice 0 keeps phase 0 so a single
    // oscillator behaves like the classic mono case.
    const int unison1 = juce::jlimit (1, maxUnisonVoices, params.osc1.unisonVoices);
    const int unison2 = juce::jlimit (1, maxUnisonVoices, params.osc2.unisonVoices);
    for (int u = 0; u < unison1; ++u)
        osc1Bank[static_cast<size_t> (u)].setPhase (static_cast<float> (u)
                                                    / static_cast<float> (unison1));
    for (int u = 0; u < unison2; ++u)
        osc2Bank[static_cast<size_t> (u)].setPhase (static_cast<float> (u)
                                                    / static_cast<float> (unison2));

    // Sub: sine locked one octave below osc1's coarse pitch.
    subOsc.setWaveform (dsp::WavetableOscillator::Waveform::sine);
    subOsc.setFrequency (semitonesToFrequency (static_cast<float> (midiNote)
                                               + params.osc1.coarseSemitones - 12.0f));
    subOsc.reset();

    filter.setType (static_cast<dsp::SVFFilter::Type> (params.filterType));
    filter.setParameters (params.filterCutoff, params.filterResonance);

    ampEnv.noteOn (velocity);
}

void SynthVoice::stopNote() noexcept
{
    ampEnv.noteOff();
}

bool SynthVoice::isActive() const noexcept
{
    return ampEnv.isActive();
}

void SynthVoice::renderBank (OscBank& bank, const OscillatorParameters& oscParams,
                             float* left, float* right, int numSamples) noexcept
{
    if (oscParams.level <= 0.0001f)
        return;

    const int unison = juce::jlimit (1, maxUnisonVoices, oscParams.unisonVoices);

    // 1/sqrt(N) keeps the perceived level roughly constant as unison voices
    // (uncorrelated once detuned) are stacked.
    const float norm = oscParams.level / std::sqrt (static_cast<float> (unison));

    for (int u = 0; u < unison; ++u)
    {
        // Equal-power pan: centre voices sit at -3 dB per side, outer voices
        // move toward hard left/right as unisonSpread approaches 1.
        const float pan = unisonPosition (u, unison) * oscParams.unisonSpread; // -1..1
        const float angle = (pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
        const float gainL = std::cos (angle) * norm;
        const float gainR = std::sin (angle) * norm;

        auto& osc = bank[static_cast<size_t> (u)];
        for (int i = 0; i < numSamples; ++i)
        {
            const float sample = osc.getNextSample();
            left[i] += gainL * sample;
            right[i] += gainR * sample;
        }
    }
}

void SynthVoice::renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample,
                                  int numSamples, const SynthParameters& params) noexcept
{
    jassert (startSample >= 0 && startSample + numSamples <= buffer.getNumSamples());
    jassert (numSamples <= scratch.getNumSamples()); // sized in prepare()
    if (numSamples <= 0)
        return;

    // Push parameter updates (audio thread safe — value writes and pointer
    // swaps only). Retuning here keeps coarse/detune live on held notes.
    updateBankTuning (osc1Bank, params.osc1);
    updateBankTuning (osc2Bank, params.osc2);
    subOsc.setFrequency (semitonesToFrequency (static_cast<float> (currentNote)
                                               + params.osc1.coarseSemitones - 12.0f));

    filter.setType (static_cast<dsp::SVFFilter::Type> (params.filterType));
    filter.setParameters (params.filterCutoff, params.filterResonance);
    ampEnv.setParameters (params.ampAttack, params.ampDecay, params.ampSustain, params.ampRelease);

    auto* left = scratch.getWritePointer (0);
    auto* right = scratch.getWritePointer (1);
    juce::FloatVectorOperations::clear (left, numSamples);
    juce::FloatVectorOperations::clear (right, numSamples);

    // 1) Oscillator banks (stereo unison spread), sub and noise -> scratch.
    renderBank (osc1Bank, params.osc1, left, right, numSamples);
    renderBank (osc2Bank, params.osc2, left, right, numSamples);

    constexpr float centreGain = 0.70710678f; // match the unison centre pan law

    if (params.subLevel > 0.0001f)
    {
        const float gain = params.subLevel * centreGain;
        for (int i = 0; i < numSamples; ++i)
        {
            const float sample = subOsc.getNextSample() * gain;
            left[i] += sample;
            right[i] += sample;
        }
    }

    if (params.noiseLevel > 0.0001f)
    {
        const float gain = params.noiseLevel * centreGain;
        for (int i = 0; i < numSamples; ++i)
        {
            const float sample = nextNoiseSample() * gain;
            left[i] += sample;
            right[i] += sample;
        }
    }

    // 2) Filter (stereo, shared coefficients) and amp envelope.
    filter.processStereo (left, right, numSamples);
    ampEnv.applyEnvelopeToStereo (left, right, numSamples);

    // 3) Accumulate into the output. Mono hosts get the average; stereo (and
    //    wider) hosts get L on even channels, R on odd ones.
    if (buffer.getNumChannels() == 1)
    {
        auto* out = buffer.getWritePointer (0, startSample);
        for (int i = 0; i < numSamples; ++i)
            out[i] += 0.5f * (left[i] + right[i]);
        return;
    }

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        const float* src = (channel % 2 == 0) ? left : right;
        auto* out = buffer.getWritePointer (channel, startSample);
        for (int i = 0; i < numSamples; ++i)
            out[i] += src[i];
    }
}

} // namespace usam
