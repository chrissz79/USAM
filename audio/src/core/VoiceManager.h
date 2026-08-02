// VoiceManager
// -----------------------------------------------------------------------------
// Owns a fixed pool of SynthVoice objects (polyphony), handles note-on/off
// allocation and a simple voice-stealing policy (oldest released first, then
// oldest playing). All methods are audio-thread safe by design: the pool is
// sized in prepare() and never resized on the audio thread.
#pragma once

#include "SynthParameters.h"
#include "SynthVoice.h"

#include <vector>

namespace usam
{

class VoiceManager
{
public:
    VoiceManager() = default;
    ~VoiceManager() = default;

    VoiceManager (const VoiceManager&) = delete;
    VoiceManager& operator= (const VoiceManager&) = delete;

    /** Creates the voice pool. polyphony is capped to a sane max (64).
        maxBlockSize is forwarded to each voice so internal scratch buffers
        can hold the largest host block. */
    void prepare (double sampleRate, int polyphony, int maxBlockSize);

    /** Starts a note, stealing a voice if all are busy. Audio thread. */
    void noteOn (int midiNote, float velocity, const SynthParameters& params) noexcept;

    /** Releases every voice currently playing the given MIDI note. */
    void noteOff (int midiNote) noexcept;

    /** Releases all voices (panic). */
    void allNotesOff() noexcept;

    /** Renders all active voices into the buffer. Audio thread. */
    void renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample,
                          int numSamples, const SynthParameters& params) noexcept;

    /** Number of voices currently sounding. Audio thread. */
    int getNumActiveVoices() const noexcept;

    int getPolyphony() const noexcept { return static_cast<int> (voices.size()); }

private:
    int findFreeVoice() noexcept;
    int findVoiceToSteal() noexcept;

    // unique_ptr so voices keep their non-copyable DSP state (AudioBuffer,
    // filter state) while the pool itself stays relocatable.
    std::vector<std::unique_ptr<SynthVoice>> voices;
    double sampleRate = 44100.0;
    int roundRobinIndex = 0;
};

} // namespace usam
