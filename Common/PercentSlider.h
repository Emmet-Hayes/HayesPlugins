#pragma once
#include <JuceHeader.h>

class PercentSlider : public juce::Slider
{
public:
    PercentSlider() : juce::Slider() {}

    juce::String getTextFromValue(double value) override
    {
        return juce::String(static_cast<int>(value * 100)) + "%";
    }
};
