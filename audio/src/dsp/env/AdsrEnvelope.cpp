#include "AdsrEnvelope.h"

namespace usam::dsp
{

void AdsrEnvelope::prepare (double sampleRate)
{
    adsr.setSampleRate (sampleRate);
    reset();
}

void AdsrEnvelope::setParameters (float attack, float decay, float sustain,
                                  float release) noexcept
{
    juce::ADSR::Parameters params;
    params.attack = juce::jlimit (0.0001f, 30.0f, attack);
    params.decay = juce::jlimit (0.0001f, 30.0f, decay);
    params.sustain = juce::jlimit (0.0f, 1.0f, sustain);
    params.release = juce::jlimit (0.0001f, 30.0f, release);
    adsr.setParameters (params);
}

void AdsrEnvelope::noteOn (float velocity) noexcept
{
    level = juce::jlimit (0.0f, 1.0f, velocity);
    adsr.noteOn();
}

void AdsrEnvelope::noteOff() noexcept
{
    adsr.noteOff();
}

bool AdsrEnvelope::isActive() const noexcept
{
    return adsr.isActive();
}

void AdsrEnvelope::reset() noexcept
{
    adsr.reset();
    level = 1.0f;
}

void AdsrEnvelope::applyEnvelopeToBuffer (float* data, int numSamples) noexcept
{
    for (int i = 0; i < numSamples; ++i)
        data[i] *= adsr.getNextSample() * level;
}

} // namespace usam::dsp
