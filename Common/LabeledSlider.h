#pragma once
#include <JuceHeader.h>
#include "ModSlider.h"

using SliderAttachment = std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>;
using ButtonAttachment = std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>;

class LabeledSlider : public juce::Component
{
public:
    LabeledSlider() : slider { &sliderLabel }
    {
        sliderLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(sliderLabel);

        slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
        slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, true, 50, 20);
        addAndMakeVisible(slider);
    }

    LabeledSlider(Component* parent) : slider(&sliderLabel)
    {
        addAndMakeVisible(slider);
        slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
        slider.setPopupDisplayEnabled(true, true, parent, 0);
        slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 50, 20);

        addAndMakeVisible(sliderLabel);
        sliderLabel.setJustificationType(juce::Justification::centred);
    }

    void reset(juce::AudioProcessorValueTreeState& state, const juce::String& paramID)
    {
        sAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(state, paramID, slider));
    }

    void setLabelText(const juce::String& text)
    {
        sliderLabel.setText(text, juce::dontSendNotification);
    }


    void resized() override
    {
        auto bounds = getLocalBounds().reduced(2);
        const auto fontHeight = static_cast<int>(sliderLabel.getFont().getHeight());

        const auto labelBounds = bounds.removeFromBottom(fontHeight);
        slider.setBounds(bounds);
        sliderLabel.setBounds(labelBounds);
    }

    void setHasModifier(bool mod) { slider.setHasModifiers(mod); }
    bool hasModifier() const { return slider.hasModifier(); }

private:
    ModSlider slider;
    juce::Label sliderLabel;
    SliderAttachment sAttachment;
    ButtonAttachment bAttachment;
};
