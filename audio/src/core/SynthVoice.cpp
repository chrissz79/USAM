#include "SynthVoice.h"

namespace usam
{

// MIDI note number -> frequency in Hz (A4 = 440 Hz, note 69)
static float midiNoteToFrequency (int midiNote) noexcept
{
    return 440.0f * std::pow (2.0f, static_cast<float> (midiNote - 69) / 12.0f);
}

void SynthVoice::prepare (double newSampleRate, int maxBlockSize)
{
    sampleRate = newSampleRate;

    // The scratch buffer must be able to hold the largest host block.
    scratch.setSize (1, juce::jmax (1, maxBlockSize));

    osc.prepare (sampleRate);
    filter.prepare (sampleRate);
    ampEnv.prepare (sampleRate);
    reset();
}

void SynthVoice::reset() noexcept
{
    osc.reset();
    filter.reset();
    ampEnv.reset();
    scratch.clear();
    currentNote = -1;
    velocity = 0.0f;
}

void SynthVoice::startNote (int midiNote, float newVelocity, const SynthParameters& params) noexcept
{
    currentNote = midiNote;
    velocity = juce::jlimit (0.0f, 1.0f, newVelocity);

    const float detuneSemitones = params.oscDetune / 100.0f;
    noteFrequency = midiNoteToFrequency (midiNote) * std::pow (2.0f, detuneSemitones / 12.0f);

    osc.setFrequency (noteFrequency);
    osc.reset();

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

void SynthVoice::renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample,
                                  int numSamples, const SynthParameters& params) noexcept
{
    jassert (startSample >= 0 && startSample + numSamples <= buffer.getNumSamples());
    jassert (numSamples <= scratch.getNumSamples()); // sized in prepare()
    if (numSamples <= 0)
        return;

    // Push parameter updates (audio thread safe — plain value writes).
    osc.setFrequency (noteFrequency);
    filter.setType (static_cast<dsp::SVFFilter::Type> (params.filterType));
    filter.setParameters (params.filterCutoff, params.filterResonance);
    ampEnv.setParameters (params.ampAttack, params.ampDecay, params.ampSustain, params.ampRelease);

    // 1) Oscillator -> mono scratch (osc advances once per sample).
    auto* scratchData = scratch.getWritePointer (0);
    osc.process (scratch, 0, numSamples);
    for (int i = 0; i < numSamples; ++i)
        scratchData[i] *= params.oscLevel;

    // 2) Filter (mono, one pass).
    filter.processMono (scratchData, numSamples);

    // 3) Amp envelope (advances once per sample, then spread to channels).
    ampEnv.applyEnvelopeToBuffer (scratchData, numSamples);

    // 4) Accumulate the same voice signal into every output channel.
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* outData = buffer.getWritePointer (channel, startSample);
        for (int i = 0; i < numSamples; ++i)
            outData[i] += scratchData[i];
    }
}

} // namespace usam
