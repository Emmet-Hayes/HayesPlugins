#pragma once

#include <JuceHeader.h>
#include "../../../HayesDistortionAudioProcessor.h"
#include "../../../GUI/CustomLookAndFeel.h"
#include "../../../DSP/ClippingFunctions.h"
#include "GraphTemplate.h"

class DistortionGraph  : public GraphTemplate, juce::AudioProcessorParameter::Listener, juce::Timer
{
public:
    DistortionGraph(HayesDistortionAudioProcessor &);
    ~DistortionGraph() override;

    void paint (juce::Graphics&) override;
    void setState(int mode, float rec, float mix, float bias, float drive, float rateDivide);

    void parameterValueChanged (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override { }
    void timerCallback() override;
private:
    HayesDistortionAudioProcessor &processor;
    int mode = -1;
    float color = -1.0f;
    float rec = -1.0f;
    float mix = -1.0f;
    float bias = -1.0f;
    float rateDivide = -1.0f;
    float drive = -1.0f;
    float scale = 1.0f;
    juce::Atomic<bool> parametersChanged {false};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionGraph)
};
