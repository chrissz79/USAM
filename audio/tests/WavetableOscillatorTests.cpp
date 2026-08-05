// WavetableOscillatorTests
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

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

            // Tables are peak-normalized to ±1; the band-limited square has
            // Gibbs ripple, so allow a little slack on the sampled peak.
            float maxAbs = 0.0f;
            for (int i = 0; i < n; ++i)
                maxAbs = juce::jmax (maxAbs, std::abs (buffer.getSample (0, i)));
            expectWithinAbsoluteError (maxAbs, 1.0f, 0.05f, "square amplitude");

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

        beginTest ("saw is band-limited at high pitch (no aliasing)");

        {
            // Render a saw whose fundamental sits exactly on an FFT bin, then
            // check that spectral energy only appears at harmonic bins. A
            // naive (non-band-limited) table folds harmonics beyond Nyquist
            // back onto non-harmonic bins, which this test rejects.
            constexpr int fftOrder = 13;
            constexpr int fftSize = 1 << fftOrder; // 8192
            constexpr int fundamentalBin = 600;    // ~3230 Hz at 44.1 kHz

            WavetableOscillator osc;
            osc.prepare (sr);
            osc.setWaveform (WavetableOscillator::Waveform::saw);
            osc.setFrequency (static_cast<float> (fundamentalBin)
                              * static_cast<float> (sr) / static_cast<float> (fftSize));

            juce::AudioBuffer<float> sawBuffer (1, fftSize);
            osc.process (sawBuffer, 0, fftSize);

            std::vector<float> fftData (static_cast<size_t> (2 * fftSize), 0.0f);
            for (int i = 0; i < fftSize; ++i)
                fftData[static_cast<size_t> (i)] = sawBuffer.getSample (0, i);

            juce::dsp::FFT fft (fftOrder);
            fft.performFrequencyOnlyForwardTransform (fftData.data());

            double harmonicEnergy = 0.0, aliasEnergy = 0.0;
            for (int bin = 1; bin < fftSize / 2; ++bin)
            {
                const double magnitude = fftData[static_cast<size_t> (bin)];
                const double energy = magnitude * magnitude;

                if (bin % fundamentalBin == 0)
                    harmonicEnergy += energy;
                else
                    aliasEnergy += energy;
            }

            expect (harmonicEnergy > 0.0, "saw produces harmonic content");
            expect (aliasEnergy < 0.001 * harmonicEnergy,
                    "non-harmonic (alias) energy is at least 30 dB below harmonic energy");
        }
    }
};

static WavetableOscillatorTests wavetableOscillatorTests;
