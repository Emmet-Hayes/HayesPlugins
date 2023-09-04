#pragma once

#include <JuceHeader.h>


class FreqSlider : public juce::Slider
{
public:
    FreqSlider() : juce::Slider() {}
    FreqSlider(juce::Slider::SliderStyle style, juce::Slider::TextEntryBoxPosition textPosition)
        : juce::Slider{ style, textPosition }
    {
    }

    juce::String getTextFromValue(double value) override
    {
        if (value >= 1000)
            return juce::String((int)value) + " Hz";
        else if (value >= 100)
            return juce::String(value, 1) + " Hz";
        else
            return juce::String(value, 2) + " Hz";
    }
};

