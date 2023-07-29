#pragma once
#include <JuceHeader.h>


class PitchSlider : public juce::Slider
{
public:
    PitchSlider() : juce::Slider() {}

    juce::String getTextFromValue(double val) override
    {
        if (isDiscrete)
        {
            if (val >= 1.943875)
                return juce::String("8va");
            if (val >= 1.834775)
                return juce::String("Maj7 ^");
            if (val >= 1.731795)
                return juce::String("min7 ^");
            if (val >= 1.634595)
                return juce::String("Maj6 ^");
            if (val >= 1.542615)
                return juce::String("min6 ^");
            if (val >= 1.456075)
                return juce::String("P5 ^");
            if (val >= 1.374525)
                return juce::String("tri ^");
            if (val >= 1.297725)
                return juce::String("P4 ^");
            if (val >= 1.224945)
                return juce::String("Maj3 ^");
            if (val >= 1.156245)
                return juce::String("min3 ^");
            if (val >= 1.090995)
                return juce::String("Maj2 ^");
            if (val >= 1.028725)
                return juce::String("min2 ^");
            if (val >= 0.973195)
                return juce::String("Unison");
            if (val >= 0.91738)
                return juce::String("min2 v");
            if (val >= 0.865895)
                return juce::String("Maj2 v");
            if (val >= 0.817295)
                return juce::String("min3 v");
            if (val >= 0.771305)
                return juce::String("Maj3 v");
            if (val >= 0.72803)
                return juce::String("P4 v");
            if (val >= 0.68726)
                return juce::String("tri v");
            if (val >= 0.64886)
                return juce::String("P5 v");
            if (val >= 0.61247)
                return juce::String("min6 v");
            if (val >= 0.57812)
                return juce::String("Maj6 v");
            if (val >= 0.5455)
                return juce::String("min7 v");
            if (val >= 0.51436)
                return juce::String("Maj7 v");
            return juce::String("8vb");
        }
        else
            return juce::String(val * 100, 1) + "%";
    }


    void changePitchMode() { isDiscrete = !isDiscrete; }

    bool isDiscrete = false;
};

