// VoiceManagerTests
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

#include "core/VoiceManager.h"
#include "core/SynthParameters.h"

using usam::VoiceManager;
using usam::SynthParameters;

class VoiceManagerTests : public juce::UnitTest
{
public:
    VoiceManagerTests() : juce::UnitTest ("VoiceManager") {}

    void runTest() override
    {
        constexpr double sr = 44100.0;
        constexpr int blockSize = 512;
        SynthParameters params;
        params.ampRelease = 0.001f; // short release so voices deactivate quickly

        beginTest ("allocates voices up to polyphony limit");

        {
            VoiceManager vm;
            vm.prepare (sr, 4, blockSize);

            for (int note = 0; note < 4; ++note)
                vm.noteOn (60 + note, 1.0f, params);

            expectEquals (vm.getNumActiveVoices(), 4, "four notes -> four active voices");
            expectEquals (vm.getPolyphony(), 4, "polyphony is capped at 4");

            // A 5th note should steal (not exceed polyphony).
            vm.noteOn (64, 1.0f, params);
            expectEquals (vm.getNumActiveVoices(), 4, "still four voices after stealing");
        }

        beginTest ("note-off releases the right voice after release completes");

        {
            VoiceManager vm;
            vm.prepare (sr, 4, blockSize);
            juce::AudioBuffer<float> buffer (2, blockSize);

            vm.noteOn (60, 1.0f, params);
            vm.noteOn (64, 1.0f, params);
            expectEquals (vm.getNumActiveVoices(), 2, "two voices active");

            vm.noteOff (60);
            // Render through the (1 ms) release stage so the released voice
            // finishes and deactivates.
            vm.renderNextBlock (buffer, 0, blockSize, params);
            expectEquals (vm.getNumActiveVoices(), 1, "one voice remains after note-off + release");

            vm.allNotesOff();
            vm.renderNextBlock (buffer, 0, blockSize, params);
            expectEquals (vm.getNumActiveVoices(), 0, "all notes off silences everything");
        }

        beginTest ("voices actually render audio");

        {
            VoiceManager vm;
            vm.prepare (sr, 4, blockSize);
            juce::AudioBuffer<float> buffer (2, blockSize);

            vm.noteOn (69, 1.0f, params); // A4 = 440 Hz
            vm.renderNextBlock (buffer, 0, blockSize, params);

            float energy = 0.0f;
            for (int i = 0; i < blockSize; ++i)
                energy += std::abs (buffer.getSample (0, i));

            expect (energy > 1.0f, "voice renders non-zero audio");
        }

        beginTest ("waveform parameter reaches the oscillator");

        {
            // Regression test: oscWaveform must be applied to the voice's
            // oscillator, not just stored in the parameter snapshot.
            VoiceManager vm;
            vm.prepare (sr, 4, blockSize);
            juce::AudioBuffer<float> buffer (1, blockSize);

            SynthParameters squareParams;
            squareParams.osc1.waveform = usam::dsp::WavetableOscillator::Waveform::square;
            squareParams.ampAttack = 0.001f;
            squareParams.ampSustain = 1.0f;
            squareParams.osc1.level = 1.0f;
            squareParams.filterCutoff = 20000.0f; // open the filter right up

            vm.noteOn (48, 1.0f, squareParams); // C3 = ~131 Hz, well below cutoff

            // Render a few blocks so the attack finishes and the filter
            // smoother reaches the 20 kHz target, then inspect the last block.
            for (int block = 0; block < 8; ++block)
            {
                buffer.clear();
                vm.renderNextBlock (buffer, 0, blockSize, squareParams);
            }

            // A square through a wide-open lowpass holds near peak most of the
            // time; a sine (the bug: waveform never applied) spends most of its
            // time well below peak. With the centre pan law (-3 dB per side),
            // mean |x| for the band-limited square is ~0.65, for a sine ~0.45
            // — 0.55 splits the difference.
            float meanAbs = 0.0f;
            for (int i = 0; i < blockSize; ++i)
                meanAbs += std::abs (buffer.getSample (0, i));
            meanAbs /= static_cast<float> (blockSize);

            expect (meanAbs > 0.55f, "output is square-shaped, not sine-shaped");
        }
    }
};

static VoiceManagerTests voiceManagerTests;
