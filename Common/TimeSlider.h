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
            return juce::String("Quarter");
        else if ((int)value == 2)
            return juce::String("8th");
        else if ((int)value == 3)
            return juce::String("8th dotted");
        else if ((int)value == 4)
            return juce::String("16th");
        else
            return juce::String("Unsure");
    }
};

