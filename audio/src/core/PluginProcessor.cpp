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
    cacheParameterPointers();
}

PluginProcessor::~PluginProcessor() = default;

// ---------------------------------------------------------------------------
// Parameter tree
// ---------------------------------------------------------------------------
juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    using namespace juce;

    AudioProcessorValueTreeState::ParameterLayout layout;

    const StringArray waveformNames { "Sine", "Saw", "Square", "Triangle" };

    // One oscillator's worth of parameters; osc1 is audible by default,
    // osc2 fades in when the user raises its level.
    auto addOscillator = [&layout, &waveformNames] (const String& prefix, const String& name,
                                                    float defaultLevel)
    {
        layout.add (std::make_unique<AudioParameterChoice> (ParameterID { prefix + "Waveform", 1 },
                                                            name + " Waveform", waveformNames, 0));
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { prefix + "Coarse", 1 },
                                                           name + " Coarse (semi)",
                                                           NormalisableRange<float> (-24.0f, 24.0f, 1.0f),
                                                           0.0f));
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { prefix + "Detune", 1 },
                                                           name + " Detune (cents)",
                                                           NormalisableRange<float> (-100.0f, 100.0f, 0.1f),
                                                           0.0f));
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { prefix + "Level", 1 },
                                                           name + " Level",
                                                           NormalisableRange<float> (0.0f, 1.0f, 0.001f),
                                                           defaultLevel));
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { prefix + "Unison", 1 },
                                                           name + " Unison",
                                                           NormalisableRange<float> (1.0f, 16.0f, 1.0f),
                                                           1.0f));
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { prefix + "UnisonDetune", 1 },
                                                           name + " Unison Detune (cents)",
                                                           NormalisableRange<float> (0.0f, 100.0f, 0.1f, 0.5f),
                                                           20.0f));
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { prefix + "UnisonSpread", 1 },
                                                           name + " Unison Spread",
                                                           NormalisableRange<float> (0.0f, 1.0f, 0.001f),
                                                           0.5f));
    };

    addOscillator ("osc1", "Osc 1", 0.8f);
    addOscillator ("osc2", "Osc 2", 0.0f);

    // Sub & noise layers
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::subLevel, 1 },
                                                       "Sub Level",
                                                       NormalisableRange<float> (0.0f, 1.0f, 0.001f),
                                                       0.0f));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::noiseLevel, 1 },
                                                       "Noise Level",
                                                       NormalisableRange<float> (0.0f, 1.0f, 0.001f),
                                                       0.0f));

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

void PluginProcessor::cacheParameterPointers()
{
    auto resolveOsc = [this] (const juce::String& prefix, OscParamRefs& refs)
    {
        refs.waveform = apvts.getRawParameterValue (prefix + "Waveform");
        refs.coarse = apvts.getRawParameterValue (prefix + "Coarse");
        refs.detune = apvts.getRawParameterValue (prefix + "Detune");
        refs.level = apvts.getRawParameterValue (prefix + "Level");
        refs.unison = apvts.getRawParameterValue (prefix + "Unison");
        refs.unisonDetune = apvts.getRawParameterValue (prefix + "UnisonDetune");
        refs.unisonSpread = apvts.getRawParameterValue (prefix + "UnisonSpread");
    };

    resolveOsc ("osc1", osc1Refs);
    resolveOsc ("osc2", osc2Refs);

    subLevelRef = apvts.getRawParameterValue (ParamID::subLevel);
    noiseLevelRef = apvts.getRawParameterValue (ParamID::noiseLevel);
    filterCutoffRef = apvts.getRawParameterValue (ParamID::filterCutoff);
    filterResonanceRef = apvts.getRawParameterValue (ParamID::filterResonance);
    filterTypeRef = apvts.getRawParameterValue (ParamID::filterType);
    ampAttackRef = apvts.getRawParameterValue (ParamID::ampAttack);
    ampDecayRef = apvts.getRawParameterValue (ParamID::ampDecay);
    ampSustainRef = apvts.getRawParameterValue (ParamID::ampSustain);
    ampReleaseRef = apvts.getRawParameterValue (ParamID::ampRelease);
    masterGainRef = apvts.getRawParameterValue (ParamID::masterGain);
}

// ---------------------------------------------------------------------------
// Audio lifecycle
// ---------------------------------------------------------------------------
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    voiceManager.prepare (sampleRate, polyphony, samplesPerBlock);
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
void PluginProcessor::snapshotOsc (const OscParamRefs& refs, OscillatorParameters& out) noexcept
{
    out.waveform = static_cast<dsp::WavetableOscillator::Waveform> (
        static_cast<int> (refs.waveform->load()));
    out.coarseSemitones = refs.coarse->load();
    out.detuneCents = refs.detune->load();
    out.level = refs.level->load();
    out.unisonVoices = static_cast<int> (refs.unison->load());
    out.unisonDetuneCents = refs.unisonDetune->load();
    out.unisonSpread = refs.unisonSpread->load();
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Rebuild the parameter snapshot from the cached atomics (lock-free).
    snapshotOsc (osc1Refs, params.osc1);
    snapshotOsc (osc2Refs, params.osc2);
    params.subLevel = subLevelRef->load();
    params.noiseLevel = noiseLevelRef->load();
    params.filterCutoff = filterCutoffRef->load();
    params.filterResonance = filterResonanceRef->load();
    params.filterType = static_cast<int> (filterTypeRef->load());
    params.ampAttack = ampAttackRef->load();
    params.ampDecay = ampDecayRef->load();
    params.ampSustain = ampSustainRef->load();
    params.ampRelease = ampReleaseRef->load();
    params.masterGain = masterGainRef->load();

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
