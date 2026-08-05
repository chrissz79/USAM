// PluginEditor
// -----------------------------------------------------------------------------
// Minimal development editor: a data-driven control panel binding every APVTS
// parameter to a labelled slider or combo box, laid out in rows. The full
// themed GUI (oscillator panel, mod matrix, etc.) is a later M1 work item.
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../core/PluginProcessor.h"

#include <memory>
#include <vector>

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

    /** Creates a labelled rotary slider bound to paramID and appends it to the
        current row. */
    void addSlider (const juce::String& paramID, const juce::String& labelText);

    /** Creates a labelled combo box bound to paramID and appends it to the
        current row. */
    void addCombo (const juce::String& paramID, const juce::String& labelText,
                   const juce::StringArray& items);

    void newRow();
    void attachLabel (juce::Component& target, const juce::String& text);

    PluginProcessor& processor;

    juce::Label titleLabel;
    juce::Label voiceCountLabel;

    std::vector<std::unique_ptr<juce::Slider>> sliders;
    std::vector<std::unique_ptr<juce::ComboBox>> combos;
    std::vector<std::unique_ptr<juce::Label>> labels;
    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<ComboAttachment>> comboAttachments;

    std::vector<std::vector<juce::Component*>> rows; // layout order

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
