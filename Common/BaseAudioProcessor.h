#pragma once
#include <JuceHeader.h>

class BaseAudioProcessor : public juce::AudioProcessor
{
public:
    BaseAudioProcessor(juce::AudioProcessorValueTreeState::ParameterLayout layout);
  
    void releaseResources() override {}
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int /*index*/) override {}
    const juce::String getProgramName(int /*index*/) override { return {}; }
    void changeProgramName(int /*index*/, const juce::String& /*newName*/) override {}
    bool hasEditor() const override { return true; }

    virtual void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    virtual void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    virtual juce::AudioProcessorEditor* createEditor() override;
    
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
  
    virtual juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() = 0;
  
    juce::AudioProcessorValueTreeState apvts;
};
