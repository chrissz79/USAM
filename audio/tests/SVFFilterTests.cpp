// SVFFilterTests
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

#include "dsp/filter/SVFFilter.h"

using usam::dsp::SVFFilter;

class SVFFilterTests : public juce::UnitTest
{
public:
    SVFFilterTests() : juce::UnitTest ("SVFFilter") {}

    void runTest() override
    {
        constexpr double sr = 44100.0;
        constexpr int n = 4096;

        auto makeSine = [&] (float freq)
        {
            juce::AudioBuffer<float> buffer (1, n);
            for (int i = 0; i < n; ++i)
                buffer.setSample (0, i, static_cast<float> (
                    std::sin (2.0 * juce::MathConstants<double>::pi * freq * i / sr)));
            return buffer;
        };

        auto rms = [&] (const juce::AudioBuffer<float>& buffer)
        {
            double sum = 0.0;
            for (int i = 0; i < n; ++i)
            {
                const double s = buffer.getSample (0, i);
                sum += s * s;
            }
            return static_cast<float> (std::sqrt (sum / n));
        };

        beginTest ("lowpass keeps lows, attenuates highs");

        {
            const auto low = makeSine (200.0f);
            const auto high = makeSine (8000.0f);

            juce::AudioBuffer<float> lowOut (low);
            juce::AudioBuffer<float> highOut (high);

            SVFFilter filter;
            filter.prepare (sr);
            filter.setType (SVFFilter::Type::lowpass);
            filter.setParameters (1000.0f, 0.7f);
            filter.process (lowOut, 0, n);

            SVFFilter filter2;
            filter2.prepare (sr);
            filter2.setType (SVFFilter::Type::lowpass);
            filter2.setParameters (1000.0f, 0.7f);
            filter2.process (highOut, 0, n);

            const float lowRms = rms (lowOut);
            const float highRms = rms (highOut);

            // At cutoff=1000, the 200 Hz tone should pass nearly untouched
            // while 8 kHz should be strongly attenuated.
            expect (lowRms > 0.5f * rms (low), "lowpass passes low frequencies");
            expect (highRms < 0.2f * rms (high), "lowpass attenuates high frequencies");
        }

        beginTest ("highpass keeps highs, attenuates lows");

        {
            const auto low = makeSine (100.0f);
            const auto high = makeSine (8000.0f);

            juce::AudioBuffer<float> lowOut (low);
            juce::AudioBuffer<float> highOut (high);

            SVFFilter filter;
            filter.prepare (sr);
            filter.setType (SVFFilter::Type::highpass);
            filter.setParameters (1000.0f, 0.7f);
            filter.process (lowOut, 0, n);

            SVFFilter filter2;
            filter2.prepare (sr);
            filter2.setType (SVFFilter::Type::highpass);
            filter2.setParameters (1000.0f, 0.7f);
            filter2.process (highOut, 0, n);

            expect (rms (lowOut) < 0.2f * rms (low), "highpass attenuates low frequencies");
            expect (rms (highOut) > 0.5f * rms (high), "highpass passes high frequencies");
        }
    }
};

static SVFFilterTests svfFilterTests;
