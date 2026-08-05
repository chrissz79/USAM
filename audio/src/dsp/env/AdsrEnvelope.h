// AdsrEnvelope
// -----------------------------------------------------------------------------
// Amplitude envelope. Wraps juce::ADSR behind a stable engine interface.
// Supports velocity scaling and tempo sync is a later-milestone feature.
#pragma once

#include <juce_dsp/juce_dsp.h>

namespace usam::dsp
{

class AdsrEnvelope
{
public:
    AdsrEnvelope() = default;
    ~AdsrEnvelope() = default;

    AdsrEnvelope (const AdsrEnvelope&) = delete;
    AdsrEnvelope& operator= (const AdsrEnvelope&) = delete;

    /** Must be called once before use (and again if sample rate changes). */
    void prepare (double sampleRate);

    /** Sets attack/decay (seconds), sustain (0..1), release (seconds). Audio thread safe. */
    void setParameters (float attack, float decay, float sustain, float release) noexcept;

    /** Triggers the note on with a velocity in 0..1 (scales the output level). */
    void noteOn (float velocity) noexcept;

    /** Triggers the release stage. */
    void noteOff() noexcept;

    /** True while the envelope is still producing sound. Audio thread safe. */
    bool isActive() const noexcept;

    /** Resets the envelope to an inactive state. Audio thread safe. */
    void reset() noexcept;

    /** Applies the envelope to a mono buffer (multiplies in place). Audio thread. */
    void applyEnvelopeToBuffer (float* data, int numSamples) noexcept;

    /** Applies the envelope to a stereo pair, advancing the envelope once per
        sample (both channels get the identical gain). Audio thread. */
    void applyEnvelopeToStereo (float* left, float* right, int numSamples) noexcept;

private:
    juce::ADSR adsr;
    float level = 1.0f; // velocity-scaled output level
};

} // namespace usam::dsp
