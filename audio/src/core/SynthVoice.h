// SynthVoice
// -----------------------------------------------------------------------------
// A single polyphonic voice: wavetable oscillator -> SVF filter -> ADSR amp.
// Owns its own DSP state so polyphony = N voices, each with independent phase,
// filter and envelope. Voice stealing is handled by the VoiceManager.
//
// Real-time safety: all buffers are pre-allocated in prepare(); renderNextBlock
// performs zero allocations, zero locks, zero I/O. The voice renders into a
// mono scratch buffer first (so the filter/envelope advance once per sample),
// then spreads the result across all output channels.
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

    /** Renders this voice into the buffer (accumulates with existing content). */
    void renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample,
                          int numSamples, const SynthParameters& params) noexcept;

    /** The MIDI note this voice currently plays (-1 when free). */
    int getCurrentNote() const noexcept { return currentNote; }

private:
    dsp::WavetableOscillator osc;
    dsp::SVFFilter filter;
    dsp::AdsrEnvelope ampEnv;

    juce::AudioBuffer<float> scratch { 1, 1 }; // sized in prepare(); mono scratch

    int currentNote = -1;
    float velocity = 0.0f;
    float noteFrequency = 0.0f;   // Hz of current note (with detune applied)
    double sampleRate = 44100.0;
};

} // namespace usam
