#include "PluginEditor.h"

namespace usam
{

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processor (p), voiceCountTimer (*this)
{
    setSize (600, 500);

    titleLabel.setText ("USAM ENGINE — M1", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (18.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (titleLabel);

    voiceCountLabel.setText ("voices: 0", juce::dontSendNotification);
    voiceCountLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible (voiceCountLabel);

    const juce::StringArray waveformNames { "Sine", "Saw", "Square", "Triangle" };

    newRow(); // Osc 1
    addCombo (ParamID::osc1Waveform, "Osc1 Wave", waveformNames);
    addSlider (ParamID::osc1Coarse, "Coarse");
    addSlider (ParamID::osc1Detune, "Detune");
    addSlider (ParamID::osc1Level, "Level");
    addSlider (ParamID::osc1Unison, "Unison");
    addSlider (ParamID::osc1UnisonDetune, "Uni Det");
    addSlider (ParamID::osc1UnisonSpread, "Spread");

    newRow(); // Osc 2
    addCombo (ParamID::osc2Waveform, "Osc2 Wave", waveformNames);
    addSlider (ParamID::osc2Coarse, "Coarse");
    addSlider (ParamID::osc2Detune, "Detune");
    addSlider (ParamID::osc2Level, "Level");
    addSlider (ParamID::osc2Unison, "Unison");
    addSlider (ParamID::osc2UnisonDetune, "Uni Det");
    addSlider (ParamID::osc2UnisonSpread, "Spread");

    newRow(); // Layers + filter
    addSlider (ParamID::subLevel, "Sub");
    addSlider (ParamID::noiseLevel, "Noise");
    addCombo (ParamID::filterType, "Filter", { "Lowpass", "Highpass", "Bandpass" });
    addSlider (ParamID::filterCutoff, "Cutoff");
    addSlider (ParamID::filterResonance, "Res");

    newRow(); // Amp env + master
    addSlider (ParamID::ampAttack, "Attack");
    addSlider (ParamID::ampDecay, "Decay");
    addSlider (ParamID::ampSustain, "Sustain");
    addSlider (ParamID::ampRelease, "Release");
    addSlider (ParamID::masterGain, "Master");

    // Refresh voice count a few times per second (UI thread only).
    voiceCountTimer.startTimerHz (4);
}

PluginEditor::~PluginEditor() = default;

void PluginEditor::newRow()
{
    rows.emplace_back();
}

void PluginEditor::attachLabel (juce::Component& target, const juce::String& text)
{
    auto label = std::make_unique<juce::Label>();
    label->setText (text, juce::dontSendNotification);
    label->setFont (juce::Font (12.0f));
    label->setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    label->setJustificationType (juce::Justification::centred);
    label->attachToComponent (&target, false); // positions itself above target
    addAndMakeVisible (*label);
    labels.push_back (std::move (label));
}

void PluginEditor::addSlider (const juce::String& paramID, const juce::String& labelText)
{
    auto slider = std::make_unique<juce::Slider> (juce::Slider::RotaryVerticalDrag,
                                                  juce::Slider::TextBoxBelow);
    slider->setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 16);
    slider->setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff4fc3f7));
    slider->setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider->setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (*slider);
    attachLabel (*slider, labelText);

    sliderAttachments.push_back (std::make_unique<SliderAttachment> (processor.getAPVTS(),
                                                                     paramID, *slider));
    rows.back().push_back (slider.get());
    sliders.push_back (std::move (slider));
}

void PluginEditor::addCombo (const juce::String& paramID, const juce::String& labelText,
                             const juce::StringArray& items)
{
    auto combo = std::make_unique<juce::ComboBox>();
    combo->addItemList (items, 1);
    addAndMakeVisible (*combo);
    attachLabel (*combo, labelText);

    comboAttachments.push_back (std::make_unique<ComboAttachment> (processor.getAPVTS(),
                                                                   paramID, *combo));
    rows.back().push_back (combo.get());
    combos.push_back (std::move (combo));
}

void PluginEditor::VoiceCountTimer::timerCallback()
{
    owner.voiceCountLabel.setText ("voices: " + juce::String (owner.processor.getNumActiveVoices()),
                                   juce::dontSendNotification);
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1e1e2e));
    g.setColour (juce::Colour (0xff313244));
    g.drawRect (getLocalBounds(), 1);
}

void PluginEditor::resized()
{
    auto bounds = getLocalBounds().reduced (12);
    titleLabel.setBounds (bounds.removeFromTop (24));
    voiceCountLabel.setBounds (bounds.removeFromTop (14));

    const int labelHeight = 16;   // attached labels sit above each control
    const int controlHeight = 78;
    const int controlWidth = 76;
    const int gap = 6;

    for (auto& row : rows)
    {
        auto rowArea = bounds.removeFromTop (labelHeight + controlHeight);
        rowArea.removeFromTop (labelHeight);

        for (auto* component : row)
        {
            auto slot = rowArea.removeFromLeft (controlWidth);
            // Combo boxes don't need the full rotary height.
            if (dynamic_cast<juce::ComboBox*> (component) != nullptr)
                slot = slot.withHeight (24).withY (slot.getY() + (controlHeight - 24) / 2);
            component->setBounds (slot);
            rowArea.removeFromLeft (gap);
        }
    }
}

} // namespace usam
