#pragma once
#include "../../../HayesDistortionAudioProcessor.h"
#include "GraphTemplate.h"
#include <JuceHeader.h>

class Oscilloscope : public GraphTemplate, juce::Timer
{
public:
    Oscilloscope (HayesDistortionAudioProcessor&);
    ~Oscilloscope() override;

    void paint (juce::Graphics&) override;
    void timerCallback() override;

private:
    HayesDistortionAudioProcessor& processor;

    juce::Array<float> historyL;
    juce::Array<float> historyR;
    juce::Image historyImage;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Oscilloscope)
};
