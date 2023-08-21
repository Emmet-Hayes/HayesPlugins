#pragma once

#include <JuceHeader.h>
#include "HayesVisualizerAudioProcessor.h"
#include "../../Common/BaseAudioProcessorEditor.h"
#include "../../Common/CustomLookAndFeel.h"
#include "../../Common/OpenGLComponent.h"

class HayesVisualizerAudioProcessorEditor  : public BaseAudioProcessorEditor
                                           , public juce::Timer
{
public:
    HayesVisualizerAudioProcessorEditor (HayesVisualizerAudioProcessor&);
    ~HayesVisualizerAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    CustomLookAndFeel customLookAndFeel;

    HayesVisualizerAudioProcessor& processor;

    std::unique_ptr<OpenGLComponent> openGLComponent;
    
    int defaultWidth = 800;
    int defaultHeight = 600;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HayesVisualizerAudioProcessorEditor)
};
