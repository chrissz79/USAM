#include "PluginProcessor.h"
#include "gui/PluginEditor.h"

// Plugin entry point required by JUCE's generated host wrappers.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new usam::PluginProcessor();
}

namespace usam
{

PluginProcessor::PluginProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    // Parameter defaults are defined in createParameterLayout(); they must mirror
    // the SynthParameters defaults. Nothing extra to set here.
}

PluginProcessor::~PluginProcessor() = default;

// ---------------------------------------------------------------------------
// Parameter tree
// ---------------------------------------------------------------------------
juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    using namespace juce;

    AudioProcessorValueTreeState::ParameterLayout layout;

    // Oscillator
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::oscWaveform, 1 },
                                                        "Osc Waveform",
                                                        StringArray { "Sine", "Saw", "Square", "Triangle" },
                                                        0));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::oscDetune, 1 },
                                                       "Osc Detune (cents)",
                                                       NormalisableRange<float> (-1200.0f, 1200.0f, 0.1f),
                                                       0.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::oscLevel, 1 },
                                                       "Osc Level",
                                                       NormalisableRange<float> (0.0f, 1.0f, 0.001f),
                                                       0.8f));

    // Filter
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::filterCutoff, 1 },
                                                       "Filter Cutoff",
                                                       NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.25f),
                                                       12000.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::filterResonance, 1 },
                                                       "Filter Resonance",
                                                       NormalisableRange<float> (0.1f, 20.0f, 0.01f, 0.3f),
                                                       0.7f));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::filterType, 1 },
                                                        "Filter Type",
                                                        StringArray { "Lowpass", "Highpass", "Bandpass" },
                                                        0));

    // Amp envelope
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::ampAttack, 1 },
                                                       "Amp Attack",
                                                       NormalisableRange<float> (0.001f, 10.0f, 0.001f, 0.3f),
                                                       0.01f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::ampDecay, 1 },
                                                       "Amp Decay",
                                                       NormalisableRange<float> (0.001f, 10.0f, 0.001f, 0.3f),
                                                       0.2f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::ampSustain, 1 },
                                                       "Amp Sustain",
                                                       NormalisableRange<float> (0.0f, 1.0f, 0.001f),
                                                       0.7f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::ampRelease, 1 },
                                                       "Amp Release",
                                                       NormalisableRange<float> (0.001f, 10.0f, 0.001f, 0.3f),
                                                       0.3f));

    // Master
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::masterGain, 1 },
                                                       "Master Gain",
                                                       NormalisableRange<float> (-60.0f, 12.0f, 0.1f),
                                                       -6.0f));

    return layout;
}

// ---------------------------------------------------------------------------
// Audio lifecycle
// ---------------------------------------------------------------------------
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    voiceManager.prepare (sampleRate, polyphony, samplesPerBlock);
    params = SynthParameters{};
}

void PluginProcessor::releaseResources()
{
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

bool PluginProcessor::hasEditor() const
{
    return true;
}

// ---------------------------------------------------------------------------
// Processing
// ---------------------------------------------------------------------------
void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Rebuild the parameter snapshot (reads APVTS — safe on the audio thread
    // as long as we do not lock the value tree; JUCE parameter access is
    // designed for this pattern). getRawParameterValue returns std::atomic,
    // so use .load() (JUCE 9).
    params.oscWaveform = static_cast<dsp::WavetableOscillator::Waveform> (
        apvts.getRawParameterValue (ParamID::oscWaveform)->load());
    params.oscDetune = apvts.getRawParameterValue (ParamID::oscDetune)->load();
    params.oscLevel = apvts.getRawParameterValue (ParamID::oscLevel)->load();
    params.filterCutoff = apvts.getRawParameterValue (ParamID::filterCutoff)->load();
    params.filterResonance = apvts.getRawParameterValue (ParamID::filterResonance)->load();
    params.filterType = static_cast<int> (apvts.getRawParameterValue (ParamID::filterType)->load());
    params.ampAttack = apvts.getRawParameterValue (ParamID::ampAttack)->load();
    params.ampDecay = apvts.getRawParameterValue (ParamID::ampDecay)->load();
    params.ampSustain = apvts.getRawParameterValue (ParamID::ampSustain)->load();
    params.ampRelease = apvts.getRawParameterValue (ParamID::ampRelease)->load();
    params.masterGain = apvts.getRawParameterValue (ParamID::masterGain)->load();

    // MIDI -> voices
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
            const auto velocity = msg.getFloatVelocity();
            if (velocity > 0.0f)
                voiceManager.noteOn (msg.getNoteNumber(), velocity, params);
            else
                voiceManager.noteOff (msg.getNoteNumber());
        }
        else if (msg.isNoteOff())
        {
            voiceManager.noteOff (msg.getNoteNumber());
        }
        else if (msg.isAllNotesOff() || msg.isAllSoundOff())
        {
            voiceManager.allNotesOff();
        }
    }

    // Render voices
    buffer.clear();
    voiceManager.renderNextBlock (buffer, 0, buffer.getNumSamples(), params);

    // Master gain
    const float gain = juce::Decibels::decibelsToGain (params.masterGain);
    buffer.applyGain (gain);
}

// ---------------------------------------------------------------------------
// State (patch save/load)
// ---------------------------------------------------------------------------
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr)
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

} // namespace usam
