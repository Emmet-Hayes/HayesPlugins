#pragma once

#include "../../../HayesDistortionAudioProcessor.h"
#include "../../../GUI/DriveLookAndFeel.h"
#include "../../../DSP/ClippingFunctions.h"

#include "GraphTemplate.h"


class DistortionGraph  : public GraphTemplate
                       , juce::AudioProcessorParameter::Listener
                       , juce::Timer
{
public:
    DistortionGraph(HayesDistortionAudioProcessor &);
    ~DistortionGraph() override;

    void paint (juce::Graphics&) override;
    void setState(int pmode, float prec, float pmix, float pbias, float pdrive, float prateDivide);

    void parameterValueChanged (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int /*parameterIndex*/, bool /*gestureIsStarting*/) override { }
    void timerCallback() override;
private:
    HayesDistortionAudioProcessor &processor;
    int mode         { -1 };
    float color      { -1.0f };
    float rec        { -1.0f };
    float mix        { -1.0f };
    float bias       { -1.0f };
    float rateDivide { -1.0f };
    float drive      { -1.0f };
    float scale      {  1.0f };
    juce::Atomic<bool> parametersChanged { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistortionGraph)
};
