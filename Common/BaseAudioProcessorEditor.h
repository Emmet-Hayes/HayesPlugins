#pragma once

#include <JuceHeader.h>
#include "BaseAudioProcessor.h"


class BaseAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    BaseAudioProcessorEditor(BaseAudioProcessor&);
private:
	BaseAudioProcessor& processor;
};
