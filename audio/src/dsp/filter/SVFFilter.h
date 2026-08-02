// SVFFilter
// -----------------------------------------------------------------------------
// State-variable filter (SVF) with selectable type (LP/HP/BP) and parameter
// smoothing to avoid zipper noise on cutoff/resonance changes.
//
// Backed by JUCE 9's dsp::StateVariableTPTFilter; wrapped here so the rest of
// the engine talks to a single stable interface (a custom saturation SVF
// replaces the internals in a later milestone without touching call sites).
#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace usam::dsp
{

class SVFFilter
{
public:
    enum class Type
    {
        lowpass,
        highpass,
        bandpass
    };

    SVFFilter() = default;
    ~SVFFilter() = default;

    SVFFilter (const SVFFilter&) = delete;
    SVFFilter& operator= (const SVFFilter&) = delete;

    /** Must be called once before use (and again if sample rate changes). */
    void prepare (double sampleRate);

    /** Resets filter state and smoothing. Audio thread safe. */
    void reset() noexcept;

    /** Selects filter response type. Audio thread safe. */
    void setType (Type newType) noexcept;

    /** Sets cutoff (Hz) and resonance (Q). Smoothed internally. Audio thread safe. */
    void setParameters (float cutoffHz, float resonanceQ) noexcept;

    /** Renders in place (single channel view of the buffer range). Audio thread. */
    void process (juce::AudioBuffer<float>& buffer, int startSample, int numSamples) noexcept;

    /** In-place mono helper for per-voice rendering. Audio thread. */
    void processMono (float* data, int numSamples) noexcept;

private:
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::SmoothedValue<float> cutoffSmoother;
    juce::SmoothedValue<float> resonanceSmoother;
    Type type = Type::lowpass;
};

} // namespace usam::dsp
