#include "HayesVisualizerAudioProcessor.h"
#include "HayesVisualizerAudioProcessorEditor.h"


HayesVisualizerAudioProcessor::HayesVisualizerAudioProcessor()
:   BaseAudioProcessor { createParameterLayout() }
{
}

HayesVisualizerAudioProcessor::~HayesVisualizerAudioProcessor()
{
}

void HayesVisualizerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void HayesVisualizerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
}

juce::AudioProcessorEditor* HayesVisualizerAudioProcessor::createEditor()
{
    return new HayesVisualizerAudioProcessorEditor (*this);
}

juce::AudioProcessorValueTreeState::ParameterLayout HayesVisualizerAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    return layout;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HayesVisualizerAudioProcessor();
}
