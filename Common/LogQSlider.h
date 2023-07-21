#pragma once
#include <JuceHeader.h>

class LogQSlider : public juce::Slider
{
public:
    LogQSlider() : juce::Slider() {}

    juce::String getTextFromValue(double value) override
    {
        if (value >= 1000)
            return juce::String((int)value);
        else if (value >= 100)
            return juce::String(value, 1);
        else if (value >= 10)
            return juce::String(value, 2);
        else
            return juce::String(value, 3);
    }
};

