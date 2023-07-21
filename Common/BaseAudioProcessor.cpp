#include "BaseAudioProcessor.h"
#include "BaseAudioProcessorEditor.h"

BaseAudioProcessor::BaseAudioProcessor(juce::AudioProcessorValueTreeState::ParameterLayout layout)
:   juce::AudioProcessor()
,   apvts {*this, nullptr, "PARAMETERS", std::move(layout) }
{
}

void BaseAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
}

void BaseAudioProcessor::processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&)
{
}

juce::AudioProcessorEditor* BaseAudioProcessor::createEditor()
{
	return new BaseAudioProcessorEditor(*this);
}

void BaseAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.state.createXml())
        copyXmlToBinary(*xml, destData);
}


void BaseAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.state = juce::ValueTree::fromXml(*xml);
}

