// SynthVoice
// -----------------------------------------------------------------------------
// A single polyphonic voice:
//   [osc1 unison bank + osc2 unison bank + sub osc + noise] -> SVF filter
//   -> ADSR amp, rendered in stereo.
//
// Each oscillator bank holds up to 16 unison voices with symmetric detune and
// equal-power stereo spread. The sub oscillator is a sine one octave below
// osc1's pitch; the noise layer is white noise from a per-voice LCG.
//
// Real-time safety: all buffers are pre-allocated in prepare(); renderNextBlock
// performs zero allocations, zero locks, zero I/O. The voice renders into a
// stereo scratch buffer first (so the filter/envelope advance once per
// sample), then accumulates into the output channels.
#pragma once

#include "SynthParameters.h"
#include "dsp/env/AdsrEnvelope.h"
#include "dsp/filter/SVFFilter.h"
#include "dsp/osc/WavetableOscillator.h"

#include <juce_audio_basics/juce_audio_basics.h>

namespace usam
{

class SynthVoice
{
public:
    static constexpr int maxUnisonVoices = 16;

    SynthVoice() = default;
    ~SynthVoice() = default;

    SynthVoice (const SynthVoice&) = delete;
    SynthVoice& operator= (const SynthVoice&) = delete;

    /** Prepare DSP state. Must be called before use; safe to call again.
        maxBlockSize must be >= the largest host block the plugin will see. */
    void prepare (double sampleRate, int maxBlockSize);

    /** Reset all internal state (voice released/returned to pool). */
    void reset() noexcept;

    /** Starts a note. midiNote 0..127, velocity 0..1. Audio thread. */
    void startNote (int midiNote, float velocity, const SynthParameters& params) noexcept;

    /** Enters the release stage of the amp envelope. Audio thread. */
    void stopNote() noexcept;

    /** True while the amp envelope is producing sound. Audio thread. */
    bool isActive() const noexcept;

    /** Renders this voice into the buffer (accumulates with existing content).
        Stereo: channel 0 gets the voice's left signal, channel 1 the right;
        a mono buffer receives the (L+R)/2 mixdown. */
    void renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample,
                          int numSamples, const SynthParameters& params) noexcept;

    /** The MIDI note this voice currently plays (-1 when free). */
    int getCurrentNote() const noexcept { return currentNote; }

private:
    using OscBank = std::array<dsp::WavetableOscillator, maxUnisonVoices>;

    /** Pushes waveform + per-unison-voice frequency into a bank. */
    void updateBankTuning (OscBank& bank, const OscillatorParameters& oscParams) noexcept;

    /** Renders one bank into the stereo scratch (accumulating). */
    void renderBank (OscBank& bank, const OscillatorParameters& oscParams,
                     float* left, float* right, int numSamples) noexcept;

    float nextNoiseSample() noexcept
    {
        // LCG (Numerical Recipes constants) -> uniform white noise in [-1, 1).
        noiseState = noiseState * 1664525u + 1013904223u;
        return static_cast<float> (static_cast<int32_t> (noiseState))
               * (1.0f / 2147483648.0f);
    }

    OscBank osc1Bank;
    OscBank osc2Bank;
    dsp::WavetableOscillator subOsc;
    dsp::SVFFilter filter;
    dsp::AdsrEnvelope ampEnv;

    juce::AudioBuffer<float> scratch { 2, 1 }; // sized in prepare(); stereo scratch

    uint32_t noiseState = 0x9e3779b9u;
    int currentNote = -1;
    float velocity = 0.0f;
    double sampleRate = 44100.0;
};

} // namespace usam
