#include "BandPanel.h"


BandPanel::BandPanel(HayesDistortionAudioProcessor& p) 
:   processor { p }
{
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        compressorBypassButtons[i] = std::make_unique<juce::ToggleButton>();
        widthBypassButtons[i] = std::make_unique<juce::ToggleButton>();
    }
    // init vec
    shapeVector = { &recKnobs[0], &recKnobs[1], &recKnobs[2], &recKnobs[3], &biasKnobs[0], &biasKnobs[1], &biasKnobs[2], &biasKnobs[3], &shapePanelLabel };
    widthVector = { &widthKnobs[0], &widthKnobs[1], &widthKnobs[2], &widthKnobs[3], &*widthBypassButtons[0], &*widthBypassButtons[1], &*widthBypassButtons[2], &*widthBypassButtons[3], &widthPanelLabel };

    compressorVector = { &compThreshKnobs[0], &compThreshKnobs[1], &compThreshKnobs[2], &compThreshKnobs[3], &compRatioKnobs[0], &compRatioKnobs[1], &compRatioKnobs[2], &compRatioKnobs[3], &*compressorBypassButtons[0], &*compressorBypassButtons[1], &*compressorBypassButtons[2], &*compressorBypassButtons[3], &compressorPanelLabel };

    // init panel labels
    addAndMakeVisible (shapePanelLabel);
    shapePanelLabel.setLookAndFeel(&customLookAndFeel);
    shapePanelLabel.setText ("Shape", juce::dontSendNotification);
    shapePanelLabel.setFont (juce::Font (KNOB_FONT, KNOB_FONT_SIZE, juce::Font::plain));
    shapePanelLabel.setColour (juce::Label::textColourId, SHAPE_COLOUR);
    
    addAndMakeVisible (compressorPanelLabel);
    compressorPanelLabel.setLookAndFeel(&customLookAndFeel);
    compressorPanelLabel.setText ("Compressor", juce::dontSendNotification);
    compressorPanelLabel.setFont (juce::Font (KNOB_FONT, KNOB_FONT_SIZE, juce::Font::plain));
    compressorPanelLabel.setColour (juce::Label::textColourId, COMP_COLOUR);
    
    addAndMakeVisible (widthPanelLabel);
    widthPanelLabel.setLookAndFeel(&customLookAndFeel);
    widthPanelLabel.setText ("Stereo", juce::dontSendNotification);
    widthPanelLabel.setFont (juce::Font (KNOB_FONT, KNOB_FONT_SIZE, juce::Font::plain));
    widthPanelLabel.setColour (juce::Label::textColourId, WIDTH_COLOUR);
    
    // drive knobs
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        driveKnobs[i].setLookAndFeel(&customLookAndFeel);
        initListenerKnob(driveKnobs[i]);
    }

    addAndMakeVisible (driveLabel);
    driveLabel.setText ("Drive", juce::dontSendNotification);
    driveLabel.setFont (juce::Font (KNOB_FONT, KNOB_FONT_SIZE, juce::Font::plain));
    driveLabel.setColour (juce::Label::textColourId, DRIVE_COLOUR.withBrightness (0.9f));
    driveLabel.attachToComponent (&driveKnobs[0], false);
    driveLabel.setJustificationType (juce::Justification::centred);

    // output knobs
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        initListenerKnob(outputKnobs[i]);
        outputKnobs[i].setColour(juce::Slider::rotarySliderFillColourId, COLOUR1);
        outputKnobs[i].setTextValueSuffix("dB");
    }

    addAndMakeVisible (outputLabel);
    outputLabel.setText ("Output", juce::dontSendNotification);
    outputLabel.setFont (juce::Font (KNOB_FONT, KNOB_FONT_SIZE, juce::Font::plain));
    outputLabel.setColour (juce::Label::textColourId, KNOB_FONT_COLOUR);
    outputLabel.attachToComponent (&outputKnobs[0], false);
    outputLabel.setJustificationType (juce::Justification::centred);

    // mix knobs
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        initRotarySlider(mixKnobs[i], COLOUR1);
        initRotarySlider(compRatioKnobs[i], COMP_COLOUR);
    }

    addAndMakeVisible (CompRatioLabel);
    CompRatioLabel.setText ("Ratio", juce::dontSendNotification);
    CompRatioLabel.setFont (juce::Font (KNOB_FONT, KNOB_FONT_SIZE, juce::Font::plain));
    CompRatioLabel.setColour (juce::Label::textColourId, COMP_COLOUR);
    CompRatioLabel.attachToComponent (&compRatioKnobs[0], false);
    CompRatioLabel.setJustificationType (juce::Justification::centred);

    // compressor threshold knobs
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        initRotarySlider(compThreshKnobs[i], COMP_COLOUR);
        compThreshKnobs[i].setTextValueSuffix("db");
    }

    addAndMakeVisible (CompThreshLabel);
    CompThreshLabel.setText ("Threshold", juce::dontSendNotification);
    CompThreshLabel.setFont (juce::Font (KNOB_FONT, KNOB_FONT_SIZE, juce::Font::plain));
    CompThreshLabel.setColour (juce::Label::textColourId, COMP_COLOUR);
    CompThreshLabel.attachToComponent (&compThreshKnobs[0], false);
    CompThreshLabel.setJustificationType (juce::Justification::centred);

    // width knobs
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
        initRotarySlider (widthKnobs[i], WIDTH_COLOUR);

    addAndMakeVisible (widthLabel);
    widthLabel.setText ("Width", juce::dontSendNotification);
    widthLabel.setFont (juce::Font (KNOB_FONT, KNOB_FONT_SIZE, juce::Font::plain));
    widthLabel.setColour (juce::Label::textColourId, WIDTH_COLOUR);
    widthLabel.attachToComponent (&widthKnobs[0], false);
    widthLabel.setJustificationType (juce::Justification::centred);

    // bias knobs
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
        initRotarySlider (biasKnobs[i], SHAPE_COLOUR);

    addAndMakeVisible (biasLabel);
    biasLabel.setText ("Bias", juce::dontSendNotification);
    biasLabel.setFont (juce::Font (KNOB_FONT, KNOB_FONT_SIZE, juce::Font::plain));
    biasLabel.setColour (juce::Label::textColourId, SHAPE_COLOUR);
    biasLabel.attachToComponent (&biasKnobs[0], false);
    biasLabel.setJustificationType (juce::Justification::centred);

    // rec knobs
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
        initRotarySlider (recKnobs[i], SHAPE_COLOUR);

    addAndMakeVisible (recLabel);
    recLabel.setText ("Rectification", juce::dontSendNotification);
    recLabel.setFont (juce::Font (KNOB_FONT, KNOB_FONT_SIZE, juce::Font::plain));
    recLabel.setColour (juce::Label::textColourId, SHAPE_COLOUR);
    recLabel.attachToComponent (&recKnobs[0], false);
    recLabel.setJustificationType (juce::Justification::centred);

    // Linked Button
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        initFlatButton(linkedButtons[i], "linked" + std::to_string(i + 1), "L");
        initFlatButton(safeButtons[i], "safe" + std::to_string(i + 1), "S");
        initFlatButton(extremeButtons[i], "extreme" + std::to_string(i + 1), "E");

    }

    addAndMakeVisible (mixLabel);
    mixLabel.setText ("Mix", juce::dontSendNotification);
    mixLabel.setFont (juce::Font (KNOB_FONT, KNOB_FONT_SIZE, juce::Font::plain));
    mixLabel.setColour (juce::Label::textColourId, KNOB_FONT_COLOUR);
    mixLabel.attachToComponent (&mixKnobs[0], false);
    mixLabel.setJustificationType (juce::Justification::centred);

    // switches
    addAndMakeVisible (oscSwitch);
    oscSwitch.setRadioGroupId (switchButtons);
    oscSwitch.setToggleState (true, juce::dontSendNotification);
    oscSwitch.setColour (juce::ToggleButton::tickDisabledColourId, DRIVE_COLOUR.withBrightness (0.5f));
    oscSwitch.setColour (juce::ToggleButton::tickColourId, DRIVE_COLOUR.withBrightness (0.9f));
    oscSwitch.setColour (juce::ComboBox::outlineColourId, COLOUR6);
    oscSwitch.setLookAndFeel (&customLookAndFeel);
    oscSwitch.addListener (this);

    addAndMakeVisible (shapeSwitch);
    shapeSwitch.setRadioGroupId (switchButtons);
    shapeSwitch.setToggleState (false, juce::dontSendNotification);
    shapeSwitch.setColour (juce::ToggleButton::tickDisabledColourId, SHAPE_COLOUR.withBrightness (0.5f));
    shapeSwitch.setColour (juce::ToggleButton::tickColourId, SHAPE_COLOUR.withBrightness (0.9f));
    shapeSwitch.setColour (juce::ComboBox::outlineColourId, COLOUR6);
    shapeSwitch.setLookAndFeel (&customLookAndFeel);
    shapeSwitch.addListener (this);

    addAndMakeVisible (compressorSwitch);
    compressorSwitch.setRadioGroupId (switchButtons);
    compressorSwitch.setToggleState (false, juce::dontSendNotification);
    compressorSwitch.setColour (juce::ToggleButton::tickDisabledColourId, COMP_COLOUR.withBrightness (0.5f));
    compressorSwitch.setColour (juce::ToggleButton::tickColourId, COMP_COLOUR.withBrightness (0.9f));
    compressorSwitch.setColour (juce::ComboBox::outlineColourId, COLOUR6);
    compressorSwitch.setLookAndFeel (&customLookAndFeel);
    compressorSwitch.addListener (this);

    addAndMakeVisible (widthSwitch);
    widthSwitch.setRadioGroupId (switchButtons);
    widthSwitch.setToggleState (false, juce::dontSendNotification);
    widthSwitch.setColour (juce::ToggleButton::tickDisabledColourId, WIDTH_COLOUR.withBrightness (0.5f));
    widthSwitch.setColour (juce::ToggleButton::tickColourId, WIDTH_COLOUR.withBrightness (0.9f));
    widthSwitch.setColour (juce::ComboBox::outlineColourId, COLOUR6);
    widthSwitch.setLookAndFeel (&customLookAndFeel);
    widthSwitch.addListener (this);

    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        initBypassButton(*compressorBypassButtons[i], COMP_COLOUR, i);
        initBypassButton(*widthBypassButtons[i], WIDTH_COLOUR, i + 4);
    }
    
    setVisibility (shapeVector, false);
    setVisibility (compressorVector, false);
    setVisibility (widthVector, false);

    // Attachment
    using SliderAttach = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttach = juce::AudioProcessorValueTreeState::ButtonAttachment;
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        driveAttachments[i] = std::make_unique<SliderAttach>(processor.apvts, "drive" + std::to_string(i + 1), driveKnobs[i]);
        compRatioAttachments[i] = std::make_unique<SliderAttach>(processor.apvts, "compRatio" + std::to_string(i + 1), compRatioKnobs[i]);
        compThreshAttachments[i] = std::make_unique<SliderAttach>(processor.apvts, "compThresh" + std::to_string(i + 1), compThreshKnobs[i]);
        outputAttachments[i] = std::make_unique<SliderAttach>(processor.apvts, "output" + std::to_string(i + 1), outputKnobs[i]);
        mixAttachments[i] = std::make_unique<SliderAttach>(processor.apvts, "mix" + std::to_string(i + 1), mixKnobs[i]);
        biasAttachments[i] = std::make_unique<SliderAttach>(processor.apvts, "bias" + std::to_string(i + 1), biasKnobs[i]);
        recAttachments[i] = std::make_unique<SliderAttach>(processor.apvts, "rec" + std::to_string(i + 1), recKnobs[i]);
        linkedAttachments[i] = std::make_unique<ButtonAttach>(processor.apvts, "linked" + std::to_string(i + 1), linkedButtons[i]);
        safeAttachments[i] = std::make_unique<ButtonAttach>(processor.apvts, "safe" + std::to_string(i + 1), safeButtons[i]);
        extremeAttachments[i] = std::make_unique<ButtonAttach>(processor.apvts, "extreme" + std::to_string(i + 1), extremeButtons[i]);
        widthAttachments[i] = std::make_unique<SliderAttach>(processor.apvts, "width" + std::to_string(i + 1), widthKnobs[i]);
        compressorBypassAttachments[i] = std::make_unique<ButtonAttach>(processor.apvts, "compressorBypass" + std::to_string(i + 1), *compressorBypassButtons[i]);
        compBypassTemp[0] = *processor.apvts.getRawParameterValue("compressorBypass" + std::to_string(i + 1));
        widthBypassTemp[0] = *processor.apvts.getRawParameterValue("widthBypass" + std::to_string(i + 1));
        setBandKnobsStates(0, *processor.apvts.getRawParameterValue("multibandEnable" + std::to_string(i + 1)), false);
    }
}

BandPanel::~BandPanel()
{
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        driveKnobs[i].setLookAndFeel(nullptr);
        linkedButtons[i].setLookAndFeel(nullptr);
        safeButtons[i].setLookAndFeel(nullptr);
        extremeButtons[i].setLookAndFeel(nullptr);
    }
    oscSwitch.setLookAndFeel (nullptr);
    shapeSwitch.setLookAndFeel (nullptr);
    widthSwitch.setLookAndFeel (nullptr);
    compressorSwitch.setLookAndFeel (nullptr);
    shapePanelLabel.setLookAndFeel (nullptr);
    compressorPanelLabel.setLookAndFeel (nullptr);
    widthPanelLabel.setLookAndFeel (nullptr);
}

void BandPanel::paint (juce::Graphics& g)
{
    setFourComponentsVisibility (driveKnobs[0], driveKnobs[1], driveKnobs[2], driveKnobs[3], focusBandNum);
    setFourComponentsVisibility (outputKnobs[0], outputKnobs[1], outputKnobs[2], outputKnobs[3], focusBandNum);
    setFourComponentsVisibility (mixKnobs[0], mixKnobs[1], mixKnobs[2], mixKnobs[3], focusBandNum);
    setFourComponentsVisibility (linkedButtons[0], linkedButtons[1], linkedButtons[2], linkedButtons[3], focusBandNum);
    setFourComponentsVisibility (safeButtons[0], safeButtons[1], safeButtons[2], safeButtons[3], focusBandNum);
    setFourComponentsVisibility (extremeButtons[0], extremeButtons[1], extremeButtons[2], extremeButtons[3], focusBandNum);

    g.setColour (COLOUR6);
    if (! oscSwitch.getToggleState())
    {
        g.drawRect (bandKnobArea);
        g.drawRect (driveKnobArea);
    }
    g.drawRect (outputKnobArea);

    // drive reduction
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        driveLookAndFeels[i].sampleMaxValue = processor.getSampleMaxValue(SAFE_ID1);
        driveLookAndFeels[i].reductionPrecent = processor.getReductionPrecent(SAFE_ID1);
    }
}

void BandPanel::resized()
{
    juce::Rectangle<int> controlArea = getLocalBounds();

    bandKnobArea = controlArea.removeFromLeft (getWidth() / 5 * 2);
    driveKnobArea = controlArea.removeFromLeft (getWidth() / 5);

    outputKnobArea = controlArea;
    driveKnobArea.removeFromTop (getHeight() / 5);
    driveKnobArea.removeFromBottom (getHeight() / 5);
    outputKnobArea.removeFromTop (getHeight() / 5);
    outputKnobArea.removeFromBottom (getHeight() / 5);
    bandKnobArea.removeFromTop (getHeight() / 5);
    bandKnobArea.removeFromBottom (getHeight() / 5);

    juce::Rectangle<int> switchArea = bandKnobArea.removeFromLeft (getWidth() / 20);
    oscSwitch.setBounds (switchArea.removeFromTop (bandKnobArea.getHeight() / 4));
    shapeSwitch.setBounds (switchArea.removeFromTop (bandKnobArea.getHeight() / 4));
    compressorSwitch.setBounds (switchArea.removeFromTop (bandKnobArea.getHeight() / 4));
    widthSwitch.setBounds (switchArea.removeFromTop (bandKnobArea.getHeight() / 4));

    juce::Rectangle<int> bigDriveArea = getLocalBounds().removeFromLeft (getWidth() / 5 * 3).reduced (getHeight() / 10);
    if (oscSwitch.getToggleState())
    {
        for (int i = 0; i < MAX_NUM_BANDS; ++i)
            driveKnobs[i].setBounds (bigDriveArea);
    }
    else
    {
        for (int i = 0; i < MAX_NUM_BANDS; ++i)
            driveKnobs[i].setBounds (driveKnobArea.reduced (0, bandKnobArea.getHeight() / 5));
    }

    // shape
    juce::Rectangle<int> bandKnobAreaLeft = bandKnobArea;
    juce::Rectangle<int> bandKnobAreaRight = bandKnobAreaLeft.removeFromRight (bandKnobArea.getWidth() / 2);
    bandKnobAreaLeft = bandKnobAreaLeft.reduced (0, bandKnobAreaLeft.getHeight() / 5);
    bandKnobAreaRight = bandKnobAreaRight.reduced (0, bandKnobAreaRight.getHeight() / 5);

    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        recKnobs[i].setBounds(bandKnobAreaLeft);
        biasKnobs[i].setBounds(bandKnobAreaRight);
    }

    // width & compressor
    juce::Rectangle<int> bypassButtonArea = bandKnobArea;
    bypassButtonArea = bypassButtonArea.removeFromBottom (bandKnobArea.getHeight() / 5).reduced (bandKnobArea.getWidth() / 2 - bandKnobArea.getHeight() / 10, 0);
    
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        widthBypassButtons[i]->setBounds(bypassButtonArea);
        widthKnobs[i].setBounds(bandKnobArea.reduced(0, bandKnobArea.getHeight() / 5));
        compressorBypassButtons[i]->setBounds(bypassButtonArea);
        compThreshKnobs[i].setBounds(bandKnobAreaLeft);
        compRatioKnobs[i].setBounds(bandKnobAreaRight);
    }
    
    // labels
    juce::Rectangle<int> panelLabelArea = bandKnobArea;
    panelLabelArea = panelLabelArea.removeFromLeft (bandKnobArea.getWidth() / 3);
    panelLabelArea = panelLabelArea.removeFromBottom (bandKnobArea.getHeight() / 5);
    shapePanelLabel.setBounds(panelLabelArea);
    compressorPanelLabel.setBounds(panelLabelArea);
    widthPanelLabel.setBounds(panelLabelArea);

    juce::Rectangle<int> outputKnobAreaLeft = outputKnobArea;
    juce::Rectangle<int> outputKnobAreaRight = outputKnobAreaLeft.removeFromRight (bandKnobArea.getWidth() / 2);
    outputKnobAreaLeft = outputKnobAreaLeft.reduced (0, outputKnobAreaLeft.getHeight() / 5);
    outputKnobAreaRight = outputKnobAreaRight.reduced (0, outputKnobAreaRight.getHeight() / 5);

    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        outputKnobs[i].setBounds(outputKnobAreaLeft);
        mixKnobs[i].setBounds(outputKnobAreaRight);
    }

    // buttons
    juce::Rectangle<int> outputButtonArea = outputKnobArea;
    outputButtonArea = outputButtonArea.removeFromLeft (getWidth() / 25);
    juce::Rectangle<int> outputButtonArea1 = outputButtonArea;
    juce::Rectangle<int> outputButtonArea2 = outputButtonArea1.removeFromBottom (outputButtonArea.getHeight() / 3 * 2);
    juce::Rectangle<int> outputButtonArea3 = outputButtonArea2.removeFromBottom (outputButtonArea.getHeight() / 3);

    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        linkedButtons[i].setBounds(outputButtonArea1);
        safeButtons[i].setBounds(outputButtonArea2);
        extremeButtons[i].setBounds(outputButtonArea3);
        driveLookAndFeels[i].scale = scale;
    }
    customLookAndFeel.setWindowScale(scale);
}

void BandPanel::sliderValueChanged (juce::Slider* slider)
{
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
        linkValue(*slider, driveKnobs[i], outputKnobs[i], linkedButtons[i]);
}

void BandPanel::linkValue (juce::Slider& xSlider, juce::Slider& driveSlider, juce::Slider& outputSlider, juce::TextButton& linkedButton)
{
    // x changes, then y will change
    if (linkedButton.getToggleState() == true)
        if (&xSlider == &driveSlider)
            outputSlider.setValue (-xSlider.getValue() * 0.1f); // use defalut notification type

}

void BandPanel::comboBoxChanged (juce::ComboBox* /*combobox*/)
{
}

void BandPanel::timerCallback()
{
}

void BandPanel::updateBypassState (juce::ToggleButton& clickedButton, int index)
{
    bool state = clickedButton.getToggleState();
    setBypassState (index, state);
}

void BandPanel::buttonClicked (juce::Button* clickedButton)
{
    juce::Rectangle<int> bigDriveArea = getLocalBounds().removeFromLeft (getWidth() / 5 * 3).reduced (getHeight() / 10);

    if (clickedButton == &oscSwitch)
    {
        if (oscSwitch.getToggleState())
        {
            for (int i = 0; i < MAX_NUM_BANDS; ++i)
                driveKnobs[i].setBounds (bigDriveArea);
            setVisibility (shapeVector, false);
            setVisibility (compressorVector, false);
            setVisibility (widthVector, false);
            recLabel.setVisible (false);
            biasLabel.setVisible (false);
            widthLabel.setVisible (false);
            CompRatioLabel.setVisible (false);
            CompThreshLabel.setVisible (false);
        }
        else
        {
            for (int i = 0; i < MAX_NUM_BANDS; ++i)
                driveKnobs[i].setBounds (driveKnobArea.reduced (0, bandKnobArea.getHeight() / 5));
        }
    }
    if (clickedButton == &shapeSwitch && shapeSwitch.getToggleState())
    {
        setVisibility(shapeVector, true);
        setFourComponentsVisibility (recKnobs[0], recKnobs[1], recKnobs[2], recKnobs[3], focusBandNum);
        setFourComponentsVisibility (biasKnobs[0], biasKnobs[1], biasKnobs[2], biasKnobs[3], focusBandNum);
        setVisibility (compressorVector, false);
        setVisibility (widthVector, false);
        CompRatioLabel.setVisible (false);
        CompThreshLabel.setVisible (false);
        recLabel.setVisible (true);
        biasLabel.setVisible (true);
        widthLabel.setVisible (false);
    }
    if (clickedButton == &compressorSwitch && compressorSwitch.getToggleState())
    {
        setVisibility (compressorVector, true);
        setFourComponentsVisibility (compRatioKnobs[0], compRatioKnobs[1], compRatioKnobs[2], compRatioKnobs[3], focusBandNum);
        setFourComponentsVisibility (compThreshKnobs[0], compThreshKnobs[1], compThreshKnobs[2], compThreshKnobs[3], focusBandNum);
        setFourComponentsVisibility (*compressorBypassButtons[0], *compressorBypassButtons[1], *compressorBypassButtons[2], *compressorBypassButtons[3], focusBandNum);
        setVisibility (shapeVector, false);
        setVisibility (widthVector, false);
        CompRatioLabel.setVisible (true);
        CompThreshLabel.setVisible (true);
        recLabel.setVisible (false);
        biasLabel.setVisible (false);
        widthLabel.setVisible (false);
    }
    if (clickedButton == &widthSwitch && widthSwitch.getToggleState())
    {
        setVisibility (widthVector, true);
        setFourComponentsVisibility (widthKnobs[0], widthKnobs[1], widthKnobs[2], widthKnobs[3], focusBandNum); // put after setVisibility
        setFourComponentsVisibility (*widthBypassButtons[0], *widthBypassButtons[1], *widthBypassButtons[2], *widthBypassButtons[3], focusBandNum);
        setVisibility (shapeVector, false);
        setVisibility (compressorVector, false);
        CompRatioLabel.setVisible (false);
        CompThreshLabel.setVisible (false);
        recLabel.setVisible (false);
        biasLabel.setVisible (false);
        widthLabel.setVisible (true);
    }
    repaint();
}

void BandPanel::initListenerKnob (juce::Slider& slider)
{
    addAndMakeVisible (slider);
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxAbove, false, TEXTBOX_WIDTH, TEXTBOX_HEIGHT);
    slider.addListener (this);
}

void BandPanel::initRotarySlider (juce::Slider& slider, juce::Colour colour)
{
    addAndMakeVisible (slider);
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxAbove, false, TEXTBOX_WIDTH, TEXTBOX_HEIGHT);
    slider.setColour (juce::Slider::rotarySliderFillColourId, colour);
}

void BandPanel::initFlatButton (juce::TextButton& button, juce::String paramId, juce::String buttonName)
{
    addAndMakeVisible (button);
    button.setClickingTogglesState (true);
    //    bool state = processor.treeState.getRawParameterValue(paramId);
    //    bool state = true;
    //    button.setToggleState(state, juce::dontSendNotification);
    button.setColour (juce::TextButton::buttonColourId, COLOUR7);
    button.setColour (juce::TextButton::buttonOnColourId, COLOUR6.withBrightness (0.1f));
    button.setColour (juce::ComboBox::outlineColourId, COLOUR6);
    button.setColour (juce::TextButton::textColourOnId, COLOUR1);
    button.setColour (juce::TextButton::textColourOffId, COLOUR7.withBrightness (0.8f));
    button.setButtonText (buttonName);
    button.setLookAndFeel (&customLookAndFeel);
}

void BandPanel::initBypassButton (juce::ToggleButton& bypassButton, juce::Colour colour, int index)
{
    addAndMakeVisible (bypassButton);
    bypassButton.setColour (juce::ToggleButton::tickColourId, colour);
    bypassButton.onClick = [this, &bypassButton, index]
    { updateBypassState (bypassButton, index); };
    updateBypassState (bypassButton, index);
    bypassButton.addListener (this);
}

void BandPanel::setFourComponentsVisibility (juce::Component& component1, juce::Component& component2, juce::Component& component3, juce::Component& component4, int bandNum)
{
    driveLabel.setVisible (true);
    mixLabel.setVisible (true);
    outputLabel.setVisible (true);
    if (bandNum == 0)
    {
        component1.setVisible (true);
        component2.setVisible (false);
        component3.setVisible (false);
        component4.setVisible (false);
    }
    else if (bandNum == 1)
    {
        component1.setVisible (false);
        component2.setVisible (true);
        component3.setVisible (false);
        component4.setVisible (false);
    }
    else if (bandNum == 2)
    {
        component1.setVisible (false);
        component2.setVisible (false);
        component3.setVisible (true);
        component4.setVisible (false);
    }
    else if (bandNum == 3)
    {
        component1.setVisible (false);
        component2.setVisible (false);
        component3.setVisible (false);
        component4.setVisible (true);
    }
}

void BandPanel::setFocusBandNum (int num)
{
    focusBandNum = num;
}

void BandPanel::setScale (float pscale)
{
    scale = pscale;
}

void BandPanel::setVisibility (juce::Array<juce::Component*>& array, bool isVisible)
{
    for (int i = 0; i < array.size(); ++i)
    {
        if (isVisible)
        {
            array[i]->setVisible (true);
        }
        else
        {
            array[i]->setVisible (false);
        }
    }
}

void BandPanel::setBypassState (int index, bool state)
{
    componentArrays[0] = { &compThreshKnobs[0], &compRatioKnobs[0] };
    componentArrays[1] = { &compThreshKnobs[1], &compRatioKnobs[1] };
    componentArrays[2] = { &compThreshKnobs[2], &compRatioKnobs[2] };
    componentArrays[3] = { &compThreshKnobs[3], &compRatioKnobs[3] };
    componentArrays[4] = { &widthKnobs[0] };
    componentArrays[5] = { &widthKnobs[1] };
    componentArrays[6] = { &widthKnobs[2] };
    componentArrays[7] = { &widthKnobs[3] };

    // 0-3 -> comp 1-4
    // 4-7 -> width 1-4
    juce::Array<juce::Component*>* componentArray = &componentArrays[index];


    // when changing subbypass buttons, also save the states to temp arrays
    if (index >= 0 && index < 4)
        compBypassTemp[index] = state;
    else if (index >= 4 && index < 8)
        widthBypassTemp[index - 4] = state;
    // set subknobs states
    for (int i = 0; i < componentArray->size(); i++)
        componentArray->data()[i]->setEnabled (state);
}

void BandPanel::setBandKnobsStates (int index, bool state, bool callFromSubBypass)
{
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
        componentArrays[0] = { &driveKnobs[i], &outputKnobs[i], &mixKnobs[i], &recKnobs[i], &biasKnobs[i], &compThreshKnobs[i], &compRatioKnobs[i], &widthKnobs[i], &linkedButtons[i], &safeButtons[i], &extremeButtons[i] };

    bool widthBypassState, compBypassState;
    // turn off bypassbuttons, subbypass buttons will also turn off
    if (! state)
    {
        widthBypassState = false;
        compBypassState = false;
    }
    else // turn on bypassbuttons, subbypass buttons states will depends on original states(temp)
    {
        widthBypassState = widthBypassTemp[index];
        compBypassState = compBypassTemp[index];
    }

    juce::Array<juce::Component*>* componentArray = &componentArrays[0];
    for (int i = 0; i < MAX_NUM_BANDS; ++i)
    {
        if (index == i)
            componentArray = &componentArrays[i];
        if (!callFromSubBypass)
        {
            widthBypassButtons[0]->setToggleState(widthBypassState, juce::NotificationType::dontSendNotification);
            compressorBypassButtons[0]->setToggleState(compBypassState, juce::NotificationType::dontSendNotification);
        }
    }

    if (state)
    {
        for (int i = 0; i < componentArray->size(); i++)
        {
            if (canEnableSubKnobs (*componentArray->data()[i]))
            {
                continue;
            }
            componentArray->data()[i]->setEnabled (true);
        }
    }
    else
    {
        for (int i = 0; i < componentArray->size(); i++)
        {
            componentArray->data()[i]->setEnabled (false);
        }
    }
}

bool BandPanel::canEnableSubKnobs (juce::Component& component)
{
    if (&component == &compThreshKnobs[0] && ! compressorBypassButtons[0]->getToggleState())
        return true;
    else if (&component == &compRatioKnobs[0] && ! compressorBypassButtons[0]->getToggleState())
        return true;
    else if (&component == &compThreshKnobs[1] && ! compressorBypassButtons[1]->getToggleState())
        return true;
    else if (&component == &compRatioKnobs[1] && ! compressorBypassButtons[1]->getToggleState())
        return true;
    else if (&component == &compThreshKnobs[2] && ! compressorBypassButtons[2]->getToggleState())
        return true;
    else if (&component == &compRatioKnobs[2] && ! compressorBypassButtons[2]->getToggleState())
        return true;
    else if (&component == &compThreshKnobs[3] && ! compressorBypassButtons[3]->getToggleState())
        return true;
    else if (&component == &compRatioKnobs[3] && ! compressorBypassButtons[3]->getToggleState())
        return true;
    else if (&component == &widthKnobs[0] && ! widthBypassButtons[0]->getToggleState())
        return true;
    else if (&component == &widthKnobs[1] && ! widthBypassButtons[1]->getToggleState())
        return true;
    else if (&component == &widthKnobs[2] && ! widthBypassButtons[2]->getToggleState())
        return true;
    else if (&component == &widthKnobs[3] && ! widthBypassButtons[3]->getToggleState())
        return true;
    else
        return false;
}

juce::ToggleButton& BandPanel::getCompButton (const int index)
{
    if (index == 0)
        return *compressorBypassButtons[0];
    else if (index == 1)
        return *compressorBypassButtons[1];
    else if (index == 2)
        return *compressorBypassButtons[2];
    else if (index == 3)
        return *compressorBypassButtons[3];
    else
        jassertfalse;
    return *compressorBypassButtons[0];
}

juce::ToggleButton& BandPanel::getWidthButton (const int index)
{
    if (index == 0)
        return *widthBypassButtons[0];
    else if (index == 1)
        return *widthBypassButtons[1];
    else if (index == 2)
        return *widthBypassButtons[2];
    else if (index == 3)
        return *widthBypassButtons[3];
    else
        jassertfalse;
    return *compressorBypassButtons[0];
}

void BandPanel::setSwitch(const int index, bool state)
{
    if (index == 0)
        oscSwitch.setToggleState(state, juce::sendNotificationSync);
    else if (index == 1)
        shapeSwitch.setToggleState(state, juce::sendNotificationSync);
    else if (index == 2)
        compressorSwitch.setToggleState(state, juce::sendNotificationSync);
    else if (index == 3)
        widthSwitch.setToggleState(state, juce::sendNotificationSync);
}

void BandPanel::updateWhenChangingFocus()
{

    if (oscSwitch.getToggleState())
    {
        setVisibility (shapeVector, false);
        setVisibility (compressorVector, false);
        setVisibility (widthVector, false);
        recLabel.setVisible (false);
        biasLabel.setVisible (false);
        widthLabel.setVisible (false);
        CompRatioLabel.setVisible (false);
        CompThreshLabel.setVisible (false);
    }

    if (shapeSwitch.getToggleState())
    {
        setFourComponentsVisibility (recKnobs[0], recKnobs[1], recKnobs[2], recKnobs[3], focusBandNum);
        setFourComponentsVisibility (biasKnobs[0], biasKnobs[1], biasKnobs[2], biasKnobs[3], focusBandNum);
        setVisibility (compressorVector, false);
        setVisibility (widthVector, false);
        CompRatioLabel.setVisible (false);
        CompThreshLabel.setVisible (false);
        recLabel.setVisible (true);
        biasLabel.setVisible (true);
        widthLabel.setVisible (false);
    }

    if (compressorSwitch.getToggleState())
    {
        setVisibility (compressorVector, true);
        setFourComponentsVisibility (compRatioKnobs[0], compRatioKnobs[1], compRatioKnobs[2], compRatioKnobs[3], focusBandNum);
        setFourComponentsVisibility (compThreshKnobs[0], compThreshKnobs[1], compThreshKnobs[2], compThreshKnobs[3], focusBandNum);
        setFourComponentsVisibility (*compressorBypassButtons[0], *compressorBypassButtons[1], *compressorBypassButtons[2], *compressorBypassButtons[3], focusBandNum);
        setVisibility (shapeVector, false);
        setVisibility (widthVector, false);
        CompRatioLabel.setVisible (true);
        CompThreshLabel.setVisible (true);
        recLabel.setVisible (false);
        biasLabel.setVisible (false);
        widthLabel.setVisible (false);
    }

    if (widthSwitch.getToggleState())
    {
        setVisibility (widthVector, true);
        setFourComponentsVisibility (widthKnobs[0], widthKnobs[1], widthKnobs[2], widthKnobs[3], focusBandNum); // put after setVisibility
        setFourComponentsVisibility (*widthBypassButtons[0], *widthBypassButtons[1], *widthBypassButtons[2], *widthBypassButtons[3], focusBandNum);
        setVisibility (shapeVector, false);
        setVisibility (compressorVector, false);
        CompRatioLabel.setVisible (false);
        CompThreshLabel.setVisible (false);
        recLabel.setVisible (false);
        biasLabel.setVisible (false);
        widthLabel.setVisible (true);
    }
    repaint();
}
