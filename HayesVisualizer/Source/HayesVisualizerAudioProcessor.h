#pragma once
#include <JuceHeader.h>
#include "../../Common/BaseAudioProcessor.h"

class HayesVisualizerAudioProcessor  : public BaseAudioProcessor
{
public:
    HayesVisualizerAudioProcessor();
    ~HayesVisualizerAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HayesVisualizerAudioProcessor)
};
