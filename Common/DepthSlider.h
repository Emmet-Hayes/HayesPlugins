#pragma once
#include <JuceHeader.h>

class DepthSlider : public juce::Slider
{
public:
    DepthSlider() : juce::Slider() {}
    DepthSlider(juce::Slider::SliderStyle style, juce::Slider::TextEntryBoxPosition textPosition)
        : juce::Slider{ style, textPosition }
    {
    }

    juce::String getTextFromValue(double value) override
    {
        return juce::String(static_cast<int>(roundToInt(value * 500))) + "%";
    }
};
