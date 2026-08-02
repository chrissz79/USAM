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

    juce::AudioProcessorValueTreeState apvts;

    VoiceManager voiceManager;
    SynthParameters params;      // snapshot rebuilt from APVTS before each block

    int polyphony = 16;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};

} // namespace usam
