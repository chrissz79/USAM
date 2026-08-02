#include "SVFFilter.h"

namespace usam::dsp
{

void SVFFilter::prepare (double sampleRate)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = 512;
    spec.numChannels = 2;

    filter.prepare (spec);
    cutoffSmoother.reset (sampleRate, 0.05);   // 50 ms smoothing time
    resonanceSmoother.reset (sampleRate, 0.05);
    reset();
}

void SVFFilter::reset() noexcept
{
    filter.reset();
    cutoffSmoother.setCurrentAndTargetValue (1000.0f);
    resonanceSmoother.setCurrentAndTargetValue (0.7f);
}

void SVFFilter::setType (Type newType) noexcept
{
    type = newType;
}

void SVFFilter::setParameters (float cutoffHz, float resonanceQ) noexcept
{
    cutoffSmoother.setTargetValue (juce::jlimit (20.0f, 20000.0f, cutoffHz));
    resonanceSmoother.setTargetValue (juce::jlimit (0.1f, 20.0f, resonanceQ));
}

void SVFFilter::processMono (float* data, int numSamples) noexcept
{
    // Apply the wrapper's filter type to the underlying JUCE filter (which
    // defaults to lowpass). This must happen before processing.
    switch (type)
    {
        case Type::lowpass:  filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);  break;
        case Type::highpass: filter.setType (juce::dsp::StateVariableTPTFilterType::highpass); break;
        case Type::bandpass: filter.setType (juce::dsp::StateVariableTPTFilterType::bandpass); break;
    }

    // Advance the smoothers to their targets for the duration of this block so
    // the filter parameters glide smoothly instead of zipper-ing.
    for (int i = 0; i < numSamples; ++i)
    {
        const float cutoff = cutoffSmoother.getNextValue();
        const float res = resonanceSmoother.getNextValue();
        filter.setCutoffFrequency (cutoff);
        filter.setResonance (res);
        data[i] = filter.processSample (0, data[i]);
    }
}

void SVFFilter::process (juce::AudioBuffer<float>& buffer, int startSample,
                         int numSamples) noexcept
{
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        processMono (buffer.getWritePointer (channel, startSample), numSamples);
}

} // namespace usam::dsp
