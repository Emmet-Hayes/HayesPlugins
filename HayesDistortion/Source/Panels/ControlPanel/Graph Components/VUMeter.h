#pragma once

#include "../../../HayesDistortionAudioProcessor.h"
#include <JuceHeader.h>

class VUMeter : public juce::Component, juce::Timer
{
public:
    VUMeter (HayesDistortionAudioProcessor* inProcessor);
    ~VUMeter() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void setParameters (bool isInput, juce::String meterName);
    void timerCallback() override;
    float getLeftChannelLevel();
    float getRightChannelLevel();

private:
    HayesDistortionAudioProcessor* mProcessor;
    bool mIsInput;
    juce::String mMeterName;
    float mCh0Level;
    float mCh1Level;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VUMeter)
};
