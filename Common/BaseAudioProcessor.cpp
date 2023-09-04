#include "BaseAudioProcessor.h"
#include "BaseAudioProcessorEditor.h"


BaseAudioProcessor::BaseAudioProcessor(juce::AudioProcessorValueTreeState::ParameterLayout layout)
:   juce::AudioProcessor()
,   apvts {*this, nullptr, "PARAMETERS", std::move(layout) }
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BaseAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void BaseAudioProcessor::prepareToPlay(double /*sampleRate*/, int /*samplesPerBlock*/)
{
}

void BaseAudioProcessor::processBlock(juce::AudioBuffer<float>& /*buffer*/, juce::MidiBuffer& /*midiMessages*/)
{
}

juce::AudioProcessorEditor* BaseAudioProcessor::createEditor()
{
	return new BaseAudioProcessorEditor(*this);
}

void BaseAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos (destData, true);
    apvts.state.writeToStream (mos);
}


void BaseAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData (data,
                                               static_cast<size_t> (sizeInBytes));

    if (tree.isValid())
        apvts.replaceState (tree);
}
