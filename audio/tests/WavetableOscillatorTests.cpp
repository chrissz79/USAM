// WavetableOscillatorTests
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

#include "dsp/osc/WavetableOscillator.h"

using usam::dsp::WavetableOscillator;

class WavetableOscillatorTests : public juce::UnitTest
{
public:
    WavetableOscillatorTests() : juce::UnitTest ("WavetableOscillator") {}

    void runTest() override
    {
        constexpr double sr = 44100.0;
        constexpr int n = 4096;
        juce::AudioBuffer<float> buffer (1, n);

        beginTest ("sine waveform is a sine at expected frequency");

        {
            WavetableOscillator osc;
            osc.prepare (sr);
            osc.setWaveform (WavetableOscillator::Waveform::sine);
            osc.setFrequency (440.0f);
            osc.process (buffer, 0, n);

            // Count zero crossings (positive-going). For a 440 Hz sine over
            // 4096 samples at 44.1 kHz we expect ~40.8 periods.
            int crossings = 0;
            for (int i = 1; i < n; ++i)
                if (buffer.getSample (0, i - 1) < 0.0f && buffer.getSample (0, i) >= 0.0f)
                    ++crossings;

            const float measuredHz = static_cast<float> (crossings) * static_cast<float> (sr)
                                     / static_cast<float> (n);
            expectWithinAbsoluteError (measuredHz, 440.0f, 10.0f, "zero-crossing frequency");

            // Peak-to-peak should be roughly 2.0
            float min = 1.0f, max = -1.0f;
            for (int i = 0; i < n; ++i)
            {
                min = juce::jmin (min, buffer.getSample (0, i));
                max = juce::jmax (max, buffer.getSample (0, i));
            }
            expectWithinAbsoluteError (max, 1.0f, 0.05f, "sine max ~1.0");
            expectWithinAbsoluteError (min, -1.0f, 0.05f, "sine min ~-1.0");
        }

        beginTest ("square waveform alternates cleanly");

        {
            WavetableOscillator osc;
            osc.prepare (sr);
            osc.setWaveform (WavetableOscillator::Waveform::square);
            osc.setFrequency (220.0f);
            osc.process (buffer, 0, n);

            // With a square table, output should be very close to ±1.
            float maxAbs = 0.0f;
            for (int i = 0; i < n; ++i)
                maxAbs = juce::jmax (maxAbs, std::abs (buffer.getSample (0, i)));
            expectWithinAbsoluteError (maxAbs, 1.0f, 0.01f, "square amplitude");

            // Both signs should appear.
            bool sawPositive = false, sawNegative = false;
            for (int i = 0; i < n; ++i)
            {
                sawPositive |= buffer.getSample (0, i) > 0.5f;
                sawNegative |= buffer.getSample (0, i) < -0.5f;
            }
            expect (sawPositive && sawNegative, "square contains both polarities");
        }

        beginTest ("phase wraps without discontinuity");

        {
            WavetableOscillator osc;
            osc.prepare (sr);
            osc.setWaveform (WavetableOscillator::Waveform::sine);
            osc.setFrequency (4410.0f); // exactly 100 periods over n samples
            osc.process (buffer, 0, n);

            // Sample-to-sample step must never exceed the sine's max slope,
            // i.e. no wraparound glitch larger than ~0.1 (2*pi*4400/44100 ~ 0.62 max
            // for a 4400 Hz sine; a glitch would jump by ~2.0).
            float maxStep = 0.0f;
            for (int i = 1; i < n; ++i)
                maxStep = juce::jmax (maxStep, std::abs (buffer.getSample (0, i)
                                                          - buffer.getSample (0, i - 1)));
            expect (maxStep < 1.5f, "no large phase-wrap discontinuity");
        }
    }
};

static WavetableOscillatorTests wavetableOscillatorTests;
