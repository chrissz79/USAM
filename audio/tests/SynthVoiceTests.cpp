// SynthVoiceTests — dual oscillator, sub, noise and unison behaviour,
// exercised through the VoiceManager (the production render path).
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

#include "core/VoiceManager.h"
#include "core/SynthParameters.h"

using usam::VoiceManager;
using usam::SynthParameters;
using Waveform = usam::dsp::WavetableOscillator::Waveform;

class SynthVoiceTests : public juce::UnitTest
{
public:
    SynthVoiceTests() : juce::UnitTest ("SynthVoice") {}

    static float measureFrequency (const juce::AudioBuffer<float>& buffer, int n, double sr)
    {
        int crossings = 0;
        for (int i = 1; i < n; ++i)
            if (buffer.getSample (0, i - 1) < 0.0f && buffer.getSample (0, i) >= 0.0f)
                ++crossings;
        return static_cast<float> (crossings) * static_cast<float> (sr) / static_cast<float> (n);
    }

    void runTest() override
    {
        constexpr double sr = 44100.0;
        constexpr int n = 4096;

        // Fast attack, full sustain, open filter: the tests below measure the
        // raw oscillator content.
        SynthParameters base;
        base.ampAttack = 0.001f;
        base.ampSustain = 1.0f;
        base.filterCutoff = 20000.0f;
        base.osc1.level = 0.0f; // each test enables exactly the layers it needs

        beginTest ("osc2 renders at its own coarse pitch");

        {
            SynthParameters params = base;
            params.osc2.waveform = Waveform::sine;
            params.osc2.level = 1.0f;
            params.osc2.coarseSemitones = 12.0f; // one octave above the note

            VoiceManager vm;
            vm.prepare (sr, 4, n);
            juce::AudioBuffer<float> buffer (2, n);
            buffer.clear();

            vm.noteOn (69, 1.0f, params); // A4 = 440 Hz -> osc2 at 880 Hz
            vm.renderNextBlock (buffer, 0, n, params);

            expectWithinAbsoluteError (measureFrequency (buffer, n, sr), 880.0f, 15.0f,
                                       "osc2 plays one octave up");
        }

        beginTest ("sub oscillator sits one octave below the note");

        {
            SynthParameters params = base;
            params.subLevel = 1.0f;

            VoiceManager vm;
            vm.prepare (sr, 4, n);
            juce::AudioBuffer<float> buffer (2, n);
            buffer.clear();

            vm.noteOn (69, 1.0f, params); // A4 -> sub at 220 Hz
            vm.renderNextBlock (buffer, 0, n, params);

            expectWithinAbsoluteError (measureFrequency (buffer, n, sr), 220.0f, 15.0f,
                                       "sub plays one octave down");
        }

        beginTest ("noise layer renders broadband noise");

        {
            SynthParameters params = base;
            params.noiseLevel = 1.0f;

            VoiceManager vm;
            vm.prepare (sr, 4, n);
            juce::AudioBuffer<float> buffer (2, n);
            buffer.clear();

            vm.noteOn (69, 1.0f, params);
            vm.renderNextBlock (buffer, 0, n, params);

            double sum = 0.0, sumAbs = 0.0, lag1 = 0.0, energy = 0.0;
            for (int i = 0; i < n; ++i)
            {
                const double s = buffer.getSample (0, i);
                sum += s;
                sumAbs += std::abs (s);
                energy += s * s;
                if (i > 0)
                    lag1 += s * buffer.getSample (0, i - 1);
            }

            expect (sumAbs / n > 0.1, "noise has audible level");
            expect (std::abs (sum / n) < 0.05, "noise is zero-centred");
            // A pitched tone at any audible frequency has |lag-1 autocorrelation|
            // near 1; white noise (even lightly lowpassed) stays far below.
            expect (std::abs (lag1 / energy) < 0.5, "noise is not periodic");
        }

        beginTest ("unison spread decorrelates left and right; spread 0 stays mono");

        {
            SynthParameters params = base;
            params.osc1.waveform = Waveform::saw;
            params.osc1.level = 1.0f;
            params.osc1.unisonVoices = 7;
            params.osc1.unisonDetuneCents = 30.0f;
            params.osc1.unisonSpread = 1.0f;

            VoiceManager vm;
            vm.prepare (sr, 4, n);
            juce::AudioBuffer<float> buffer (2, n);
            buffer.clear();

            vm.noteOn (57, 1.0f, params); // A3
            vm.renderNextBlock (buffer, 0, n, params);

            double diff = 0.0, mag = 0.0;
            for (int i = 0; i < n; ++i)
            {
                const double l = buffer.getSample (0, i);
                const double r = buffer.getSample (1, i);
                diff += std::abs (l - r);
                mag += std::abs (l) + std::abs (r);
            }

            expect (mag > 0.0, "unison stack renders audio");
            expect (diff > 0.05 * mag, "full spread produces a wide stereo image");

            // Same stack with spread 0 must collapse to identical channels.
            params.osc1.unisonSpread = 0.0f;

            VoiceManager vmMono;
            vmMono.prepare (sr, 4, n);
            juce::AudioBuffer<float> monoBuffer (2, n);
            monoBuffer.clear();

            vmMono.noteOn (57, 1.0f, params);
            vmMono.renderNextBlock (monoBuffer, 0, n, params);

            float maxChannelDelta = 0.0f;
            for (int i = 0; i < n; ++i)
                maxChannelDelta = juce::jmax (maxChannelDelta,
                                              std::abs (monoBuffer.getSample (0, i)
                                                        - monoBuffer.getSample (1, i)));
            expect (maxChannelDelta < 1.0e-6f, "zero spread keeps channels identical");
        }
    }
};

static SynthVoiceTests synthVoiceTests;
