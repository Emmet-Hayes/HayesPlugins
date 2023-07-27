#pragma once

#include <JuceHeader.h>

#include "../../HayesDistortionAudioProcessor.h"
#include "FreqTextLabel.h"
#include "VerticalLine.h"


class FreqDividerGroup : public juce::ToggleButton
                       , juce::Slider::Listener
                       , juce::Button::Listener
{
public:
    FreqDividerGroup (HayesDistortionAudioProcessor&, int index);
    ~FreqDividerGroup() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void moveToX (int lineNum, float newXPercent, float margin, std::unique_ptr<FreqDividerGroup> freqDividerGroup[]);
    
    void setDeleteState (bool deleteState);

    VerticalLine& getVerticalLine();
    
    void setFreq (float f);
    int getFreq ();
    void setScale (float scale);

    void clicked(const juce::ModifierKeys& modifiers) override;

private:
    HayesDistortionAudioProcessor& processor;
    VerticalLine verticalLine;
    float margin = 7.5f;
    float size = 15.0f;
    float width = 5.0f;
    juce::String lineStatelId = "";
    juce::String sliderFreqId = "";
    
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseEnter (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    
    void mouseDoubleClick (const juce::MouseEvent& e) override;
    void sliderValueChanged (juce::Slider* slider) override;
    void buttonClicked (juce::Button* button) override;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> multiFreqAttachment;
    
    FreqTextLabel freqTextLabel;
    CustomLookAndFeel otherLookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FreqDividerGroup)
};
