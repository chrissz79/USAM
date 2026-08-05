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

    // Steady state: coefficients are already at target, set them once and run
    // the whole block without recomputing (setCutoffFrequency/setResonance
    // recompute a tan() internally — too expensive per sample per voice).
    if (! cutoffSmoother.isSmoothing() && ! resonanceSmoother.isSmoothing())
    {
        filter.setCutoffFrequency (cutoffSmoother.getTargetValue());
        filter.setResonance (resonanceSmoother.getTargetValue());

        for (int i = 0; i < numSamples; ++i)
            data[i] = filter.processSample (0, data[i]);

        return;
    }

    // While gliding: advance the smoothers in small chunks. At 50 ms smoothing
    // time a 16-sample update interval still gives ~140 coefficient updates
    // per glide — inaudible steps, at 1/16th of the per-sample cost.
    constexpr int updateInterval = 16;

    for (int i = 0; i < numSamples; i += updateInterval)
    {
        const int chunk = juce::jmin (updateInterval, numSamples - i);

        filter.setCutoffFrequency (cutoffSmoother.skip (chunk));
        filter.setResonance (resonanceSmoother.skip (chunk));

        for (int j = 0; j < chunk; ++j)
            data[i + j] = filter.processSample (0, data[i + j]);
    }
}

void SVFFilter::processStereo (float* left, float* right, int numSamples) noexcept
{
    switch (type)
    {
        case Type::lowpass:  filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);  break;
        case Type::highpass: filter.setType (juce::dsp::StateVariableTPTFilterType::highpass); break;
        case Type::bandpass: filter.setType (juce::dsp::StateVariableTPTFilterType::bandpass); break;
    }

    if (! cutoffSmoother.isSmoothing() && ! resonanceSmoother.isSmoothing())
    {
        filter.setCutoffFrequency (cutoffSmoother.getTargetValue());
        filter.setResonance (resonanceSmoother.getTargetValue());

        for (int i = 0; i < numSamples; ++i)
        {
            left[i] = filter.processSample (0, left[i]);
            right[i] = filter.processSample (1, right[i]);
        }
        return;
    }

    constexpr int updateInterval = 16;

    for (int i = 0; i < numSamples; i += updateInterval)
    {
        const int chunk = juce::jmin (updateInterval, numSamples - i);

        filter.setCutoffFrequency (cutoffSmoother.skip (chunk));
        filter.setResonance (resonanceSmoother.skip (chunk));

        for (int j = 0; j < chunk; ++j)
        {
            left[i + j] = filter.processSample (0, left[i + j]);
            right[i + j] = filter.processSample (1, right[i + j]);
        }
    }
}

void SVFFilter::process (juce::AudioBuffer<float>& buffer, int startSample,
                         int numSamples) noexcept
{
    // Mono only — the smoothers advance per processed sample, so running a
    // second channel through processMono would give it different coefficients.
    jassert (buffer.getNumChannels() == 1);
    processMono (buffer.getWritePointer (0, startSample), numSamples);
}

} // namespace usam::dsp
