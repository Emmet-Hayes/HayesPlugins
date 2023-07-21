#pragma once
#include <JuceHeader.h>

class LogMsSlider : public juce::Slider
{
public:
    LogMsSlider() : juce::Slider() {}

    juce::String getTextFromValue(double value) override
    {
        if (value >= 1000)
            return juce::String((int)value) + " ms";
        else if (value >= 100)
            return juce::String(value, 1) + " ms";
        else if (value >= 10)
            return juce::String(value, 2) + " ms";
        else if (value >= 1)
            return juce::String(value, 3) + " ms";
        else
            return juce::String(value, 4) + " ms";
    }
};

