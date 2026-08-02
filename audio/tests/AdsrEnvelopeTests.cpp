// AdsrEnvelopeTests
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

#include "dsp/env/AdsrEnvelope.h"

using usam::dsp::AdsrEnvelope;

class AdsrEnvelopeTests : public juce::UnitTest
{
public:
    AdsrEnvelopeTests() : juce::UnitTest ("AdsrEnvelope") {}

    void runTest() override
    {
        constexpr double sr = 44100.0;

        // applyEnvelopeToBuffer multiplies in place, so the buffer must be
        // filled with 1.0 to observe the raw envelope values.
        auto makeUnitBuffer = [] (size_t n)
        {
            std::vector<float> data (n, 1.0f);
            return data;
        };

        beginTest ("attack reaches ~1.0 then sustains");

        {
            AdsrEnvelope env;
            env.prepare (sr);
            env.setParameters (0.01f, 0.05f, 0.8f, 0.1f);
            env.noteOn (1.0f);

            auto data = makeUnitBuffer (4096);
            env.applyEnvelopeToBuffer (data.data(), static_cast<int> (data.size()));

            // Attack is 0.01 s = 441 samples; by sample 512 the envelope should
            // be at (or near) its peak.
            expectWithinAbsoluteError (data[512], 1.0f, 0.05f, "envelope reaches peak after attack");

            // Decay 0.05 s ends at ~2646; well past that, the envelope holds
            // at the sustain level.
            expectWithinAbsoluteError (data[3500], 0.8f, 0.05f, "envelope sustains at sustain level");
            expect (env.isActive(), "envelope active while sustaining");
        }

        beginTest ("release decays to zero and deactivates");

        {
            AdsrEnvelope env;
            env.prepare (sr);
            env.setParameters (0.001f, 0.01f, 0.8f, 0.05f);
            env.noteOn (1.0f);

            auto data = makeUnitBuffer (1024);
            env.applyEnvelopeToBuffer (data.data(), 1024); // through attack+decay+sustain

            env.noteOff();
            // Release is 0.05 s = 2205 samples; render well beyond that so the
            // envelope completes and deactivates.
            auto releaseData = makeUnitBuffer (8192);
            env.applyEnvelopeToBuffer (releaseData.data(), static_cast<int> (releaseData.size()));

            expect (releaseData[8191] < 0.01f, "envelope tail after release is ~0");
            expect (! env.isActive(), "envelope deactivates after release completes");
        }
    }
};

static AdsrEnvelopeTests adsrEnvelopeTests;
