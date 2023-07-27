#pragma once

#include <JuceHeader.h>

#include "../../GUI/CustomLookAndFeel.h"


class GlobalPanel : public juce::Component,
                    public juce::Slider::Listener,
                    public juce::ComboBox::Listener,
                    public juce::Timer,
                    public juce::Button::Listener
{
public:
    GlobalPanel (juce::AudioProcessorValueTreeState& apvts);
    ~GlobalPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    void setScale (float scale);
    juce::Slider& getLowcutFreqKnob();
    juce::Slider& getPeakFreqKnob();
    juce::Slider& getHighcutFreqKnob();
    juce::Slider& getLowcutGainKnob();
    juce::Slider& getPeakGainKnob();
    juce::Slider& getHighcutGainKnob();

    void setToggleButtonState (juce::String toggleButton);

private:
    void sliderValueChanged (juce::Slider* slider) override;
    void comboBoxChanged (juce::ComboBox* combobox) override;
    void buttonClicked (juce::Button* clickedButton) override;
    void setRotarySlider (juce::Slider& slider, juce::Colour colour);
    void setRoundButton (juce::TextButton& button, juce::String paramId, juce::String buttonName);
    void setBypassState (int index, bool state);
    void setVisibility (juce::Array<juce::Component*>& array, bool isVisible);

    enum RadioButtonIds
    {
        
        filterStateButtons  = 1001, // filter state: off, pre, post
        filterModeButtons   = 1002, // filter mode: low, band, high
        switchButtonsGlobal = 1005  // switches global
    };

    float scale { 1.0f };

    juce::Rectangle<int> globalEffectArea;
    juce::Rectangle<int> outputKnobArea;

    // Sliders
    juce::Slider
        downSampleKnob,
        lowcutFreqKnob,
        lowcutQKnob,
        lowcutGainKnob,
        highcutFreqKnob,
        highcutQKnob,
        highcutGainKnob,
        peakFreqKnob,
        peakQKnob,
        peakGainKnob,
        limiterThreshKnob,
        limiterReleaseKnob,
        mixKnob,
        outputKnob;

    // Labels
    juce::Label
        downSampleLabel,
        outputLabelGlobal,
        mixLabelGlobal,
        lowcutFreqLabel,
        lowcutQLabel,
        lowcutGainLabel,
        highcutFreqLabel,
        highcutQLabel,
        highcutGainLabel,
        peakFreqLabel,
        peakQLabel,
        peakGainLabel,
        filterStateLabel,
        filterTypeLabel,
        lowcutSlopeLabel,
        highcutSlopeLabel,
        limiterThreshLabel,
        limiterReleaseLabel,
    
        postFilterPanelLabel,
        downSamplePanelLabel,
        limiterPanelLabel;

    // Buttons
    juce::TextButton
        filterLowPassButton,
        filterPeakButton,
        filterHighPassButton;
    
    // switches
    juce::ToggleButton
        filterSwitch,
        downsampleSwitch,
        limiterSwitch;
    
    // vectors for sliders
    juce::Array<juce::Component*>
        filterVector,
        downsampleVector,
        limiterVector;

    std::unique_ptr<juce::ToggleButton>
        filterBypassButton,
        downsampleBypassButton,
        limiterBypassButton;

    // create own knob style
    CustomLookAndFeel otherLookAndFeel;
    CustomLookAndFeel roundedButtonLnf;
    DriveLookAndFeel driveLookAndFeel1;
    DriveLookAndFeel driveLookAndFeel2;
    DriveLookAndFeel driveLookAndFeel3;
    DriveLookAndFeel driveLookAndFeel4;
    LowPassButtonLnf lowPassButtonLnf;
    BandPassButtonLnf bandPassButtonLnf;
    HighPassButtonLnf highPassButtonLnf;
    CustomLookAndFeel flatButtonLnf;

    // Slider attachment
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        outputAttachment,
        mixAttachment,
        downSampleAttachment,
        lowcutFreqAttachment,
        lowcutQAttachment,
        lowcutGainAttachment,
        highcutFreqAttachment,
        highcutQAttachment,
        highcutGainAttachment,
        peakFreqAttachment,
        peakQAttachment,
        peakGainAttachment,
        limiterThreshAttachment,
        limiterReleaseAttachment;

    // Button attachment
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
        filterLowAttachment,
        filterBandAttachment,
        filterHighAttachment,
        filterBypassAttachment,
        downsampleBypassAttachment,
        limiterBypassAttachment;

    // ComboBox
    juce::ComboBox
        lowcutSlopeMode,
        highcutSlopeMode;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
        lowcutModeAttachment,
        highcutModeAttachment;
    
    CustomLookAndFeel flatLnf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalPanel)
};
