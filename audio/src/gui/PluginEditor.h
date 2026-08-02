// PluginEditor
// -----------------------------------------------------------------------------
// Minimal M0 editor: a compact control panel binding the APVTS parameters to
// sliders and combo boxes. A full themed GUI (oscillator panel, mod matrix,
// etc.) arrives in M1.
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../core/PluginProcessor.h"

namespace usam
{

class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    PluginProcessor& processor;

    juce::Label titleLabel;
    juce::Label voiceCountLabel;

    // Oscillator
    juce::ComboBox oscWaveformCombo;
    juce::Slider oscDetuneSlider { juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow };
    juce::Slider oscLevelSlider { juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow };

    // Filter
    juce::ComboBox filterTypeCombo;
    juce::Slider filterCutoffSlider { juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow };
    juce::Slider filterResSlider { juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow };

    // Amp envelope
    juce::Slider ampAttackSlider { juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow };
    juce::Slider ampDecaySlider { juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow };
    juce::Slider ampSustainSlider { juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow };
    juce::Slider ampReleaseSlider { juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow };

    // Master
    juce::Slider masterGainSlider { juce::Slider::RotaryVerticalDrag, juce::Slider::TextBoxBelow };

    std::unique_ptr<ComboAttachment> oscWaveformAttachment;
    std::unique_ptr<SliderAttachment> oscDetuneAttachment;
    std::unique_ptr<SliderAttachment> oscLevelAttachment;
    std::unique_ptr<ComboAttachment> filterTypeAttachment;
    std::unique_ptr<SliderAttachment> filterCutoffAttachment;
    std::unique_ptr<SliderAttachment> filterResAttachment;
    std::unique_ptr<SliderAttachment> ampAttackAttachment;
    std::unique_ptr<SliderAttachment> ampDecayAttachment;
    std::unique_ptr<SliderAttachment> ampSustainAttachment;
    std::unique_ptr<SliderAttachment> ampReleaseAttachment;
    std::unique_ptr<SliderAttachment> masterGainAttachment;

    // Periodic voice-count refresh. Timer must be subclassed (abstract callback).
    // Note: reads voice state from the UI thread while the audio thread mutates
    // it — a benign data race by JUCE convention (count is only for display).
    class VoiceCountTimer : public juce::Timer
    {
    public:
        explicit VoiceCountTimer (PluginEditor& editor) : owner (editor) {}
        void timerCallback() override;
    private:
        PluginEditor& owner;
    };

    VoiceCountTimer voiceCountTimer;
};

} // namespace usam
