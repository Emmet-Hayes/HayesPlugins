#pragma once

#include <JuceHeader.h>


class PercentSlider : public juce::Slider
{
public:
    PercentSlider() : juce::Slider() {}

    juce::String getTextFromValue(double value) override
    {
        if (value > 1.0)
            return juce::String(static_cast<int>(value)) + "%";
        return juce::String(static_cast<int>(value * 100)) + "%";
    }
};
