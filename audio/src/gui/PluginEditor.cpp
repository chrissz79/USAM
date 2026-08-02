#include "PluginEditor.h"

namespace usam
{

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processor (p), voiceCountTimer (*this)
{
    setSize (640, 360);

    titleLabel.setText ("USAM ENGINE — M0", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (18.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (titleLabel);

    voiceCountLabel.setText ("voices: 0", juce::dontSendNotification);
    voiceCountLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible (voiceCountLabel);

    auto makeCombo = [this] (juce::ComboBox& combo, const juce::StringArray& items)
    {
        combo.addItemList (items, 1);
        addAndMakeVisible (combo);
    };

    auto makeSlider = [this] (juce::Slider& slider)
    {
        slider.setSliderStyle (juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
        slider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xff4fc3f7));
        slider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
        slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible (slider);
    };

    makeCombo (oscWaveformCombo, { "Sine", "Saw", "Square", "Triangle" });
    makeSlider (oscDetuneSlider);
    makeSlider (oscLevelSlider);
    makeCombo (filterTypeCombo, { "Lowpass", "Highpass", "Bandpass" });
    makeSlider (filterCutoffSlider);
    makeSlider (filterResSlider);
    makeSlider (ampAttackSlider);
    makeSlider (ampDecaySlider);
    makeSlider (ampSustainSlider);
    makeSlider (ampReleaseSlider);
    makeSlider (masterGainSlider);

    // Bind to APVTS
    auto& apvts = processor.getAPVTS();
    oscWaveformAttachment = std::make_unique<ComboAttachment> (apvts, ParamID::oscWaveform, oscWaveformCombo);
    oscDetuneAttachment = std::make_unique<SliderAttachment> (apvts, ParamID::oscDetune, oscDetuneSlider);
    oscLevelAttachment = std::make_unique<SliderAttachment> (apvts, ParamID::oscLevel, oscLevelSlider);
    filterTypeAttachment = std::make_unique<ComboAttachment> (apvts, ParamID::filterType, filterTypeCombo);
    filterCutoffAttachment = std::make_unique<SliderAttachment> (apvts, ParamID::filterCutoff, filterCutoffSlider);
    filterResAttachment = std::make_unique<SliderAttachment> (apvts, ParamID::filterResonance, filterResSlider);
    ampAttackAttachment = std::make_unique<SliderAttachment> (apvts, ParamID::ampAttack, ampAttackSlider);
    ampDecayAttachment = std::make_unique<SliderAttachment> (apvts, ParamID::ampDecay, ampDecaySlider);
    ampSustainAttachment = std::make_unique<SliderAttachment> (apvts, ParamID::ampSustain, ampSustainSlider);
    ampReleaseAttachment = std::make_unique<SliderAttachment> (apvts, ParamID::ampRelease, ampReleaseSlider);
    masterGainAttachment = std::make_unique<SliderAttachment> (apvts, ParamID::masterGain, masterGainSlider);

    // Refresh voice count a few times per second (UI thread only).
    voiceCountTimer.startTimerHz (4);
}

void PluginEditor::VoiceCountTimer::timerCallback()
{
    owner.voiceCountLabel.setText ("voices: " + juce::String (owner.processor.getNumActiveVoices()),
                                   juce::dontSendNotification);
}

PluginEditor::~PluginEditor() = default;

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
    voiceCountLabel.setBounds (bounds.removeFromTop (16));

    const int rowHeight = 92;
    const int controlWidth = 68;

    auto row1 = bounds.removeFromTop (rowHeight);
    oscWaveformCombo.setBounds (row1.removeFromLeft (controlWidth));
    row1.removeFromLeft (6);
    oscDetuneSlider.setBounds (row1.removeFromLeft (controlWidth));
    row1.removeFromLeft (6);
    oscLevelSlider.setBounds (row1.removeFromLeft (controlWidth));
    row1.removeFromLeft (6);
    filterTypeCombo.setBounds (row1.removeFromLeft (controlWidth));
    row1.removeFromLeft (6);
    filterCutoffSlider.setBounds (row1.removeFromLeft (controlWidth));
    row1.removeFromLeft (6);
    filterResSlider.setBounds (row1.removeFromLeft (controlWidth));

    auto row2 = bounds.removeFromTop (rowHeight);
    ampAttackSlider.setBounds (row2.removeFromLeft (controlWidth));
    row2.removeFromLeft (6);
    ampDecaySlider.setBounds (row2.removeFromLeft (controlWidth));
    row2.removeFromLeft (6);
    ampSustainSlider.setBounds (row2.removeFromLeft (controlWidth));
    row2.removeFromLeft (6);
    ampReleaseSlider.setBounds (row2.removeFromLeft (controlWidth));
    row2.removeFromLeft (6);
    masterGainSlider.setBounds (row2.removeFromLeft (controlWidth));
}

} // namespace usam
