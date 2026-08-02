// WavetableOscillator
// -----------------------------------------------------------------------------
// A single-cycle wavetable oscillator with:
//   - Fixed-size wavetable (2048 samples) built from a base waveform (sine, saw,
//     square, triangle).
//   - Linear interpolation between table samples for smooth pitch.
//   - Phase accumulator with configurable frequency (Hz) and phase offset.
//   - Audio-rate phase modulation input (FM) for later cross-modulation.
//
// Real-time safety: the table is built on the UI/prepare thread (setWaveform,
// setSampleRate). The audio thread only reads the table and advances the phase.
// No allocations or locks on the audio thread.
#pragma once

#include <juce_dsp/juce_dsp.h>

namespace usam::dsp
{

class WavetableOscillator
{
public:
    enum class Waveform
    {
        sine,
        saw,
        square,
        triangle,
        count
    };

    static constexpr int tableSize = 2048;

    WavetableOscillator() = default;
    ~WavetableOscillator() = default;

    WavetableOscillator (const WavetableOscillator&) = default;
    WavetableOscillator& operator= (const WavetableOscillator&) = default;

    /** Must be called once before use (and again if sample rate changes). */
    void prepare (double sampleRate);

    /** Selects the base waveform and rebuilds the wavetable. UI thread only. */
    void setWaveform (Waveform newWaveform);

    /** Sets the oscillator frequency in Hz. Audio thread safe. */
    void setFrequency (float frequencyHz) noexcept;

    /** Sets a phase offset in radians (0..2pi). Audio thread safe. */
    void setPhaseOffset (float phaseRadians) noexcept;

    /** Audio-rate phase modulation input in radians per sample. Audio thread safe. */
    void setPhaseModulation (float phaseModRadiansPerSample) noexcept;

    /** Resets phase to zero. Audio thread safe. */
    void reset() noexcept;

    /** Renders numSamples into the buffer. Audio thread safe. */
    void process (juce::AudioBuffer<float>& buffer, int startSample, int numSamples) noexcept;

    /** Returns the next single sample (mono helper for per-voice rendering). */
    float getNextSample() noexcept;

private:
    void buildTable (Waveform waveform);

    std::array<float, tableSize> table{};
    double sampleRate = 44100.0;
    double phase = 0.0;
    double phaseIncrement = 0.0;
    float phaseOffset = 0.0f;            // in table indices (0..tableSize)
    float phaseMod = 0.0f;               // audio-rate mod in table indices
    Waveform waveform = Waveform::sine;

    static constexpr double twoPi = 6.28318530717958647692;
};

} // namespace usam::dsp
