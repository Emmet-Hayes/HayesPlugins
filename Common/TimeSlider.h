#pragma once
#include <JuceHeader.h>

class TimeSlider : public juce::Slider
{
public:
    TimeSlider() : juce::Slider() {}
    TimeSlider(juce::Slider::SliderStyle style, juce::Slider::TextEntryBoxPosition textPosition)
        : juce::Slider{ style, textPosition }
    {
    }

    juce::String getTextFromValue(double value) override
    {
        if ((int)value == 1)
            return juce::String("1/4");
        else if ((int)value == 2)
            return juce::String("1/8");
        else if ((int)value == 3)
            return juce::String("1/8 dotted");
        else if ((int)value == 4)
            return juce::String("1/16");
        else
            return juce::String("Unsure");
    }
};

