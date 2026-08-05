// WavetableOscillator
// -----------------------------------------------------------------------------
// A band-limited, mipmapped single-cycle wavetable oscillator:
//   - Fixed-size wavetables (2048 samples) built from a base waveform (sine,
//     saw, square, triangle).
//   - 11 mip levels per waveform: level L contains harmonics up to 2^L. The
//     level is selected from the playback frequency so no harmonic crosses
//     Nyquist — saw/square/triangle stay alias-free across the MIDI range.
//   - Linear interpolation between table samples for smooth pitch.
//   - Phase accumulator with configurable frequency (Hz) and phase offset.
//   - Audio-rate phase modulation input (FM) for later cross-modulation.
//
// Real-time safety: all tables are prebuilt once (additive synthesis) and
// shared immutably across every oscillator instance, so setWaveform and the
// mip switch inside setFrequency are just pointer swaps and are safe on the
// audio thread. The audio thread only reads table data and advances the
// phase. No allocations or locks on the audio thread.
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
    static constexpr int numMipLevels = 11; // level L caps harmonics at 2^L (1..1024)

    WavetableOscillator();
    ~WavetableOscillator() = default;

    WavetableOscillator (const WavetableOscillator&) = default;
    WavetableOscillator& operator= (const WavetableOscillator&) = default;

    /** Must be called once before use (and again if sample rate changes). */
    void prepare (double sampleRate);

    /** Selects the base waveform (pointer swap into the shared, prebuilt
        table set). Audio thread safe. */
    void setWaveform (Waveform newWaveform) noexcept;

    /** Sets the oscillator frequency in Hz and picks the band-limited mip
        level for it. Audio thread safe. */
    void setFrequency (float frequencyHz) noexcept;

    /** Sets a phase offset in radians (0..2pi). Audio thread safe. */
    void setPhaseOffset (float phaseRadians) noexcept;

    /** Audio-rate phase modulation input in radians per sample. Audio thread safe. */
    void setPhaseModulation (float phaseModRadiansPerSample) noexcept;

    /** Resets phase to zero. Audio thread safe. */
    void reset() noexcept;

    /** Sets the phase directly (0..1 of a cycle) — used to fan out unison
        voice phases at note start. Audio thread safe. */
    void setPhase (float normalisedPhase) noexcept;

    /** Renders numSamples into the buffer. Audio thread safe. */
    void process (juce::AudioBuffer<float>& buffer, int startSample, int numSamples) noexcept;

    /** Returns the next single sample (mono helper for per-voice rendering). */
    float getNextSample() noexcept;

private:
    using Table = std::array<float, tableSize>;

    // One immutable table per (waveform, mip level), built exactly once
    // (thread-safe magic static) the first time an oscillator is constructed.
    struct SharedTables
    {
        SharedTables();
        std::array<std::array<Table, numMipLevels>,
                   static_cast<size_t> (Waveform::count)> tables;
    };
    static const SharedTables& getSharedTables();

    int mipLevelForFrequency (float frequencyHz) const noexcept;
    void updateTablePointer() noexcept;

    const Table* table = nullptr; // points into the shared table set
    double sampleRate = 44100.0;
    double phase = 0.0;
    double phaseIncrement = 0.0;
    float frequency = 0.0f;              // Hz, as last set
    float phaseOffset = 0.0f;            // in table indices (0..tableSize)
    float phaseMod = 0.0f;               // audio-rate mod in table indices
    int mipLevel = numMipLevels - 1;     // most band-limited until a frequency is set
    Waveform waveform = Waveform::sine;

    static constexpr double twoPi = 6.28318530717958647692;
};

} // namespace usam::dsp
