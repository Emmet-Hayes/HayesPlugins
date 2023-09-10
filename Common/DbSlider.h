#pragma once

#include <JuceHeader.h>


class DbSlider : public juce::Slider
{
public:
    DbSlider() : juce::Slider() {}
  
    juce::String getTextFromValue(double value) override
    {
        if (value >= 1000)
            return juce::String((int)value) + " dB";
        else if (value >= 100 || value <= -100)
            return juce::String(value, 1) + " dB";
        else
            return juce::String(value, 2) + " dB";
    }
};
