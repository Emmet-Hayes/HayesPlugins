#pragma once

#include <JuceHeader.h>

#include "../../Common/BaseAudioProcessorEditor.h"
#include "GUI/DriveLookAndFeel.h"
#include "GUI/InterfaceDefines.h"
#include "Panels/ControlPanel/Graph Components/DistortionGraph.h"
#include "Panels/ControlPanel/Graph Components/Oscilloscope.h"
#include "Panels/ControlPanel/Graph Components/VUPanel.h"
#include "Panels/ControlPanel/Graph Components/WidthGraph.h"
#include "Panels/ControlPanel/Graph Components/GraphPanel.h"
#include "Panels/ControlPanel/BandPanel.h"
#include "Panels/ControlPanel/GlobalPanel.h"
#include "Panels/SpectrogramPanel/FilterControl.h"
#include "Panels/SpectrogramPanel/Multiband.h"


class HayesDistortionAudioProcessorEditor : public BaseAudioProcessorEditor
                                          , public juce::Slider::Listener
                                          , public juce::ComboBox::Listener
                                          , public juce::Timer
                                          , public juce::Button::Listener
{
public:
    HayesDistortionAudioProcessorEditor (HayesDistortionAudioProcessor&);
    ~HayesDistortionAudioProcessorEditor();

    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    void setMultiband();

private:
    // create own knob style
    CustomLookAndFeel customLookAndFeel;
    CustomLookAndFeel zoomLookAndFeel;

    HayesDistortionAudioProcessor& processor;
    
    int focusIndex = 0;
    void updateWhenChangingFocus();

    void buttonClicked (juce::Button* clickedButton) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void sliderValueChanged(juce::Slider* slider) override;
    void comboBoxChanged(juce::ComboBox* combobox) override;

    void setMenu(juce::ComboBox* combobox);
    void setDistortionGraph(juce::String modeId, juce::String driveId, juce::String recId, juce::String mixId, juce::String biasId, juce::String safeId);
    void setFourComponentsVisibility(juce::Component& component1, juce::Component& component2, juce::Component& component3, juce::Component& component4, int bandNum);


    GraphPanel graphPanel { processor };
    Multiband multiband { processor };
    BandPanel bandPanel { processor };
    GlobalPanel globalPanel;
    FilterControl filterControl { processor, globalPanel };
    SpectrumComponent spectrum;

    // Labels
    juce::Label hqLabel;

    // Buttons
    juce::TextButton
        hqButton,
        windowLeftButton,
        windowRightButton,
        zoomButton;

    // group toggle buttons
    enum RadioButtonIds
    {
        // window selection: left, right
        windowButtons = 1003,
    };


    // Button attachment
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>
        hqAttachment;

    // ComboBox attachment
    juce::ComboBox distortionMode1;
    juce::ComboBox distortionMode2;
    juce::ComboBox distortionMode3;
    juce::ComboBox distortionMode4;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment1;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment2;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment3;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment4;


    juce::Image image;
    PresetBar presetBar;

    float scale { 1.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HayesDistortionAudioProcessorEditor)
};
