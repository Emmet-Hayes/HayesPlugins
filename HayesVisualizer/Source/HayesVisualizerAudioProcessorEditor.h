#pragma once

#include "../../Common/BaseAudioProcessorEditor.h"
#include "../../Common/CustomLookAndFeel.h"
#include "../../Common/OpenGLComponent.h"

#include "HayesVisualizerAudioProcessor.h"


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
    juce::Label errorComponent;
    
    static constexpr int defaultWidth  { 800 };
    static constexpr int defaultHeight { 600 };
    
    bool isOpenGLAvailable { false };
    int tryInitializeOpenGL { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HayesVisualizerAudioProcessorEditor)
};
