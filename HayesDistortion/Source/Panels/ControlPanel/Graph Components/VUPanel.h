#pragma once

#include <JuceHeader.h>

#include "../../../HayesDistortionAudioProcessor.h"
#include "VUMeter.h"
#include "GraphTemplate.h"


class VUPanel  : public GraphTemplate, juce::Timer
{
public:
    VUPanel(HayesDistortionAudioProcessor &);

    void paint (juce::Graphics&) override;
    void resized() override;
    void setFocusBandNum(int num);
    void timerCallback() override;
    
private:
    HayesDistortionAudioProcessor &processor;
    int focusBandNum;
    VUMeter vuMeterIn;
    VUMeter vuMeterOut;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VUPanel)
};
