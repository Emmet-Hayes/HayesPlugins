#pragma once

#include <JuceHeader.h>

#include "../../../HayesDistortionAudioProcessor.h"
#include "GraphTemplate.h"


class WidthGraph : public GraphTemplate, juce::Timer
{
public:
    WidthGraph (HayesDistortionAudioProcessor&);

    void paint (juce::Graphics&) override;
    void timerCallback() override;
    void resized() override;

private:
    HayesDistortionAudioProcessor& processor;
    juce::Array<float> historyL;
    juce::Array<float> historyR;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WidthGraph)
};
