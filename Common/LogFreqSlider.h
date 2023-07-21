#pragma once
#include <JuceHeader.h>

class LogFreqSlider : public juce::Slider
{
public:
    LogFreqSlider() : juce::Slider() {}

    juce::String getTextFromValue(double value) override
    {
        if (value >= 1000)
            return juce::String((int)value) + " Hz";
        else if (value >= 100)
            return juce::String(value, 1) + " Hz";
        else if (value >= 10)
            return juce::String(value, 2) + " Hz";
        else
            return juce::String(value, 3) + " Hz";
    }
};

