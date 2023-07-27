#pragma once

#include "../../../HayesDistortionAudioProcessor.h"
#include "GraphTemplate.h"
#include <JuceHeader.h>

class WidthGraph : public GraphTemplate, juce::Timer
{
public:
    WidthGraph (HayesDistortionAudioProcessor&);
    ~WidthGraph() override;

    void paint (juce::Graphics&) override;
    void timerCallback() override;
    void resized() override;

private:
    HayesDistortionAudioProcessor& processor;
    juce::Array<float> historyL;
    juce::Array<float> historyR;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WidthGraph)
};
