// PluginProcessor
// -----------------------------------------------------------------------------
// The JUCE AudioProcessor for the USAM Engine. Owns the APVTS parameter tree
// and the VoiceManager, and translates MIDI + host automation into the plain
// SynthParameters snapshot consumed by the audio thread.
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "SynthParameters.h"
#include "VoiceManager.h"

namespace usam
{

class PluginProcessor : public juce::AudioProcessor
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    // -- AudioProcessor interface -------------------------------------------
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override { return "USAM Engine"; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 1.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // -- USAM-specific -------------------------------------------------------
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    const SynthParameters& getSynthParameters() const noexcept { return params; }
    int getNumActiveVoices() const noexcept { return voiceManager.getNumActiveVoices(); }

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Raw atomics for every parameter, resolved once in the constructor so
    // processBlock never does string lookups on the audio thread.
    struct OscParamRefs
    {
        std::atomic<float>* waveform = nullptr;
        std::atomic<float>* coarse = nullptr;
        std::atomic<float>* detune = nullptr;
        std::atomic<float>* level = nullptr;
        std::atomic<float>* unison = nullptr;
        std::atomic<float>* unisonDetune = nullptr;
        std::atomic<float>* unisonSpread = nullptr;
    };

    void cacheParameterPointers();
    void snapshotOsc (const OscParamRefs& refs, OscillatorParameters& out) noexcept;

    juce::AudioProcessorValueTreeState apvts;

    OscParamRefs osc1Refs, osc2Refs;
    std::atomic<float>* subLevelRef = nullptr;
    std::atomic<float>* noiseLevelRef = nullptr;
    std::atomic<float>* filterCutoffRef = nullptr;
    std::atomic<float>* filterResonanceRef = nullptr;
    std::atomic<float>* filterTypeRef = nullptr;
    std::atomic<float>* ampAttackRef = nullptr;
    std::atomic<float>* ampDecayRef = nullptr;
    std::atomic<float>* ampSustainRef = nullptr;
    std::atomic<float>* ampReleaseRef = nullptr;
    std::atomic<float>* masterGainRef = nullptr;

    VoiceManager voiceManager;
    SynthParameters params;      // snapshot rebuilt from APVTS before each block

    int polyphony = 16;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};

} // namespace usam
