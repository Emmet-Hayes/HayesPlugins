#pragma once

#include <JuceHeader.h>

#include "../../../HayesDistortionAudioProcessor.h"
#include "GraphTemplate.h"


class Oscilloscope : public GraphTemplate, juce::Timer
{
public:
    Oscilloscope (HayesDistortionAudioProcessor&);
    void paint (juce::Graphics&) override;
    void timerCallback() override;

private:
    HayesDistortionAudioProcessor& processor;

    juce::Array<float> historyL;
    juce::Array<float> historyR;
    juce::Image historyImage;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Oscilloscope)
};
