#pragma once

#include "../../GUI/DriveLookAndFeel.h"
#include "../../HayesDistortionAudioProcessor.h"


constexpr int NUM_COMPONENT_ARRAYS = 8;

class BandPanel : public juce::Component
                , public juce::Slider::Listener
                , public juce::ComboBox::Listener
                , public juce::Timer
                , public juce::Button::Listener
{
public:
    BandPanel (HayesDistortionAudioProcessor&);
    ~BandPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void setFocusBandNum(int num);
    void setScale(float scale);
    void setBandKnobsStates(int index, bool state, bool callFromSubBypass);
    juce::ToggleButton& getCompButton(const int index);
    juce::ToggleButton& getWidthButton(const int index);
    void setSwitch(const int index, bool state);
    void updateWhenChangingFocus();
private:
    HayesDistortionAudioProcessor& processor;
    juce::Rectangle<int> bandKnobArea;
    juce::Rectangle<int> driveKnobArea;
    juce::Rectangle<int> outputKnobArea;

    // override listener functions
    // linked
    void sliderValueChanged (juce::Slider* slider) override;
    // combobox changed and set knob enable/disable
    void comboBoxChanged (juce::ComboBox* combobox) override;
    void buttonClicked (juce::Button* clickedButton) override;
    void initListenerKnob (juce::Slider& slider);
    void initRotarySlider (juce::Slider& slider, juce::Colour colour);
    void initFlatButton (juce::TextButton& button, juce::String paramId, juce::String buttonName);
    void initBypassButton (juce::ToggleButton& bypassButton, juce::Colour colour, int index);
    void setFourComponentsVisibility (juce::Component& component1, juce::Component& component2, juce::Component& component3, juce::Component& component4, int bandNum);
    void linkValue (juce::Slider& xSlider, juce::Slider& driveSlider, juce::Slider& outputSlider, juce::TextButton& linkedButton);
    void setVisibility (juce::Array<juce::Component*>& array, bool isVisible);
    void updateBypassState (juce::ToggleButton& clickedButton, int index);
    void setBypassState (int index, bool state);
    bool canEnableSubKnobs (juce::Component& component);

    enum RadioButtonIds
    {
        // switches band
        switchButtons = 1004,
    };

    juce::Slider
        driveKnobs[MAX_NUM_BANDS],
        outputKnobs[MAX_NUM_BANDS],
        mixKnobs[MAX_NUM_BANDS],
        recKnobs[MAX_NUM_BANDS],
        biasKnobs[MAX_NUM_BANDS],
        compRatioKnobs[MAX_NUM_BANDS],
        compThreshKnobs[MAX_NUM_BANDS],
        widthKnobs[MAX_NUM_BANDS];

    juce::Label
        driveLabel,
        CompRatioLabel,
        CompThreshLabel,
        widthLabel,
        outputLabel,
        recLabel,
        mixLabel,
        linkedLabel,
        safeLabel,
        biasLabel,
        shapePanelLabel,
        compressorPanelLabel,
        widthPanelLabel;

    juce::TextButton
        linkedButtons[MAX_NUM_BANDS],
        safeButtons[MAX_NUM_BANDS],
        extremeButtons[MAX_NUM_BANDS];

    std::unique_ptr<juce::ToggleButton>
        compressorBypassButtons[MAX_NUM_BANDS],
        widthBypassButtons[MAX_NUM_BANDS];

    bool compBypassTemp[MAX_NUM_BANDS] = { false };
    bool widthBypassTemp[MAX_NUM_BANDS] = { false };

    // switches
    juce::ToggleButton
        oscSwitch,
        shapeSwitch,
        widthSwitch,
        compressorSwitch;

    // vectors for sliders
    juce::Array<juce::Component*>
        shapeVector,
        widthVector,
        compressorVector,
        oscVector,
        componentArrays[8];

    // Slider attachment
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        driveAttachments[MAX_NUM_BANDS],
        outputAttachments[MAX_NUM_BANDS],
        mixAttachments[MAX_NUM_BANDS],
        compRatioAttachments[MAX_NUM_BANDS],
        compThreshAttachments[MAX_NUM_BANDS],
        widthAttachments[MAX_NUM_BANDS],
        recAttachments[MAX_NUM_BANDS],
        biasAttachments[MAX_NUM_BANDS];

    // Button attachment
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
        linkedAttachments[MAX_NUM_BANDS],
        safeAttachments[MAX_NUM_BANDS],
        extremeAttachments[MAX_NUM_BANDS];

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
        compressorBypassAttachments[MAX_NUM_BANDS],
        widthBypassAttachments[MAX_NUM_BANDS];

    // conform to custom  style
    DriveLookAndFeel driveLookAndFeels[MAX_NUM_BANDS];
    CustomLookAndFeel customLookAndFeel;

    int focusBandNum;
    float scale = 1.0f;

    float tempDriveValue[4] = { 1, 1, 1, 1 };
    float tempBiasValue[4] = { 0, 0, 0, 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BandPanel)
};
