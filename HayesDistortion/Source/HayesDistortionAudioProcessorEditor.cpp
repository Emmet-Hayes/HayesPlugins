#include "HayesDistortionAudioProcessor.h"
#include "HayesDistortionAudioProcessorEditor.h"
#include "Panels/ControlPanel/Graph Components/VUMeter.h"

//==============================================================================
HayesDistortionAudioProcessorEditor::HayesDistortionAudioProcessorEditor(HayesDistortionAudioProcessor& p)
:   BaseAudioProcessorEditor (p)
,   presetBar    { p }
,   processor    { p }
,   globalPanel  { processor.apvts }
,   multiband    { p }
,   bandPanel    { p }
,   graphPanel   { p }
{
    // timer
    juce::Timer::startTimerHz (60.0f);

    // Graph
    addAndMakeVisible (graphPanel);
    graphPanel.addMouseListener (this, true);

    addAndMakeVisible (bandPanel);
    addAndMakeVisible (globalPanel);

    // Spectrum
    addAndMakeVisible(spectrum);
    addAndMakeVisible(multiband);
    multiband.addMouseListener(this, false);
    updateWhenChangingFocus();
    addAndMakeVisible(filterControl);

    for (int i = 0; i < 4; i++)
    {
        multiband.getEnableButton (i).addListener (this);
        bandPanel.getCompButton (i).addListener (this);
        bandPanel.getWidthButton (i).addListener (this);
    }

    spectrum.setInterceptsMouseClicks (false, false);
    spectrum.prepareToPaintSpectrum (processor.getNumBins(), processor.getFFTData(), processor.getSampleRate() / (float) processor.getFFTSize());

    setLookAndFeel (&otherLookAndFeel);

    // Window Left Button
    addAndMakeVisible (windowLeftButton);
    windowLeftButton.setClickingTogglesState (true);
    windowLeftButton.setRadioGroupId (windowButtons);
    windowLeftButton.setButtonText ("Band Effect");
    windowLeftButton.setToggleState (true, juce::NotificationType::dontSendNotification);
    windowLeftButton.setColour (juce::TextButton::buttonColourId, COLOUR6.withAlpha (0.5f));
    windowLeftButton.setColour (juce::TextButton::buttonOnColourId, COLOUR7);
    windowLeftButton.setColour (juce::ComboBox::outlineColourId, COLOUR1.withAlpha (0.0f));
    windowLeftButton.setColour (juce::TextButton::textColourOnId, COLOUR1);
    windowLeftButton.setColour (juce::TextButton::textColourOffId, juce::Colours::darkgrey);
    windowLeftButton.setLookAndFeel (&otherLookAndFeel);
    windowLeftButton.addListener (this);

    // Window Right Button
    addAndMakeVisible (windowRightButton);
    windowRightButton.setClickingTogglesState (true);
    windowRightButton.setRadioGroupId (windowButtons);
    windowRightButton.setButtonText ("Global Effect");
    windowRightButton.setToggleState (false, juce::NotificationType::dontSendNotification);
    windowRightButton.setColour (juce::TextButton::buttonColourId, COLOUR6.withAlpha (0.5f));
    windowRightButton.setColour (juce::TextButton::buttonOnColourId, COLOUR7);
    windowRightButton.setColour (juce::ComboBox::outlineColourId, COLOUR1.withAlpha (0.0f));
    windowRightButton.setColour (juce::TextButton::textColourOnId, COLOUR1);
    windowRightButton.setColour (juce::TextButton::textColourOffId, juce::Colours::darkgrey);
    windowRightButton.setLookAndFeel (&otherLookAndFeel);
    windowRightButton.addListener (this);

    if (windowLeftButton.getToggleState())
    {
        multiband.setVisible (true);
        bandPanel.setVisible (true);
        filterControl.setVisible (false);
        globalPanel.setVisible (false);
    }

    if (windowRightButton.getToggleState())
    {
        multiband.setVisible (false);
        bandPanel.setVisible (false);
        filterControl.setVisible (true);
        globalPanel.setVisible (true);
    }

    setMenu (&distortionMode1);
    setMenu (&distortionMode2);
    setMenu (&distortionMode3);
    setMenu (&distortionMode4);

    // zoom button
    addAndMakeVisible (zoomButton);
    zoomButton.setClickingTogglesState (false);
    //    zoomButton.setButtonText("Z");
    zoomButton.addListener (this);
    zoomButton.setColour (juce::TextButton::buttonColourId, COLOUR5.withAlpha (0.5f));
    zoomButton.setColour (juce::TextButton::buttonOnColourId, COLOUR5.withAlpha (0.5f));
    zoomButton.setColour (juce::ComboBox::outlineColourId, COLOUR5.withAlpha (0.5f));
    zoomButton.setColour (juce::TextButton::textColourOnId, COLOUR1);
    zoomButton.setColour (juce::TextButton::textColourOffId, COLOUR1.withAlpha (0.5f));
    zoomButton.setLookAndFeel (&zoomLookAndFeel);

    // use global lookandfeel
    getLookAndFeel().setColour (juce::ComboBox::textColourId, KNOB_SUBFONT_COLOUR);
    getLookAndFeel().setColour (juce::ComboBox::arrowColourId, KNOB_SUBFONT_COLOUR);
    getLookAndFeel().setColour (juce::ComboBox::buttonColourId, COLOUR1);
    getLookAndFeel().setColour (juce::ComboBox::outlineColourId, COLOUR6);
    getLookAndFeel().setColour (juce::ComboBox::focusedOutlineColourId, COLOUR1);
    getLookAndFeel().setColour (juce::ComboBox::backgroundColourId, COLOUR6);
    getLookAndFeel().setColour (juce::PopupMenu::textColourId, KNOB_SUBFONT_COLOUR);
    getLookAndFeel().setColour (juce::PopupMenu::highlightedBackgroundColourId, COLOUR5);
    getLookAndFeel().setColour (juce::PopupMenu::highlightedTextColourId, COLOUR1);
    getLookAndFeel().setColour (juce::PopupMenu::headerTextColourId, KNOB_SUBFONT_COLOUR);
    getLookAndFeel().setColour (juce::PopupMenu::backgroundColourId, juce::Colours::transparentWhite);

    modeAttachment1 = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (processor.apvts, MODE_ID1, distortionMode1);
    modeAttachment2 = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (processor.apvts, MODE_ID2, distortionMode2);
    modeAttachment3 = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (processor.apvts, MODE_ID3, distortionMode3);
    modeAttachment4 = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (processor.apvts, MODE_ID4, distortionMode4);

    presetBar.setLookAndFeel(&otherLookAndFeel);
    addAndMakeVisible(presetBar);

    // set resize
    setResizable (true, true);
    setSize (processor.getSavedWidth(), processor.getSavedHeight());
    // resize limit
    setResizeLimits (INIT_WIDTH, INIT_HEIGHT, 2000, 1000); // set resize limits
    getConstrainer()->setFixedAspectRatio (2); // set fixed resize rate

    setMultiband();
}

HayesDistortionAudioProcessorEditor::~HayesDistortionAudioProcessorEditor()
{
    stopTimer();

    setLookAndFeel (nullptr); // if this is missing - YOU WILL HIT THE ASSERT 2020/6/28
    windowRightButton.setLookAndFeel (nullptr);
    windowLeftButton.setLookAndFeel (nullptr);
    zoomButton.setLookAndFeel (nullptr);
}

void HayesDistortionAudioProcessorEditor::paint (juce::Graphics& g)
{
    int part1 = getHeight() / 10;
    int part2 = part1 * 3;

    // background
    g.setColour (COLOUR7);
    g.fillRect (0.0f, getHeight() * 3 / 10.0f, static_cast<float> (getWidth()), getHeight() * 7 / 10.0f);

    auto frame = getLocalBounds();
    frame.setBounds (0, part1, getWidth(), part2);

    int focusIndex = 0;
    focusIndex = multiband.getFocusIndex();

    if (focusIndex == 0)
    {
        setDistortionGraph (MODE_ID1, DRIVE_ID1, REC_ID1, MIX_ID1, BIAS_ID1, SAFE_ID1);
    }
    else if (focusIndex == 1)
    {
        setDistortionGraph (MODE_ID2, DRIVE_ID2, REC_ID2, MIX_ID2, BIAS_ID2, SAFE_ID2);
    }
    else if (focusIndex == 2)
    {
        setDistortionGraph (MODE_ID3, DRIVE_ID3, REC_ID3, MIX_ID3, BIAS_ID3, SAFE_ID3);
    }
    else if (focusIndex == 3)
    {
        setDistortionGraph (MODE_ID4, DRIVE_ID4, REC_ID4, MIX_ID4, BIAS_ID4, SAFE_ID4);
    }

    setFourComponentsVisibility (distortionMode1, distortionMode2, distortionMode3, distortionMode4, focusIndex);

    bool left = windowLeftButton.getToggleState();
    bool right = windowRightButton.getToggleState();

    if (left)
    { // if you select the left window, you will see audio wave and distortion function graphs.
        bandPanel.setFocusBandNum (focusIndex);
        graphPanel.setFocusBandNum (focusIndex);
    }
    else if (right)
    {
        graphPanel.setFocusBandNum (-1); // -1 means global
    }
}

void HayesDistortionAudioProcessorEditor::resized()
{
    processor.setSavedHeight (getHeight());
    processor.setSavedWidth (getWidth());

    // knobs
    float scale = juce::jmin (getHeight() / INIT_HEIGHT, getWidth() / INIT_WIDTH);
    float scaleMax = juce::jmax (getHeight() / INIT_HEIGHT, getWidth() / INIT_WIDTH);

    // top bar
    juce::Rectangle<int> area (getLocalBounds());
    juce::Rectangle<int> topBar = area.removeFromTop (50 * getHeight() / INIT_HEIGHT);
    presetBar.setBounds (topBar);

    // spectrum and filter
    if (zoomButton.getToggleState())
    {
        juce::Rectangle<int> spectrumArea = area;
        spectrum.setBounds (spectrumArea);
        multiband.setBounds (spectrumArea);
        filterControl.setBounds (spectrumArea);
    }
    else
    {
        juce::Rectangle<int> spectrumArea = area.removeFromBottom (SPEC_HEIGHT);
        spectrum.setBounds (spectrumArea);
        multiband.setBounds (spectrumArea);
        filterControl.setBounds (spectrumArea);

        // left and right window buttons
        float windowHeight = getHeight() / 20;
        juce::Rectangle<int> leftWindowButtonArea = area.removeFromTop (windowHeight);
        juce::Rectangle<int> rightWindowButtonArea = leftWindowButtonArea.removeFromRight (getWidth() / 2);
        windowLeftButton.setBounds (leftWindowButtonArea);
        windowRightButton.setBounds (rightWindowButtonArea);

        area.removeFromLeft (getWidth() / 20);
        area.removeFromRight (getWidth() / 20);

        juce::Rectangle<int> controlArea = area;
        juce::Rectangle<int> controlAreaTop = area.removeFromTop (area.getHeight() / 5);
        juce::Rectangle<int> controlAreaMid = area.removeFromTop (area.getHeight() / 4 * 3); // 3/4

        // distortion menu
        juce::Rectangle<int> distortionModeArea = controlAreaTop.removeFromLeft (OSC_WIDTH); // width
        distortionModeArea.removeFromTop (controlAreaTop.getHeight() / 4);
        distortionModeArea.removeFromBottom (controlAreaTop.getHeight() / 4);
        distortionMode1.setBounds (distortionModeArea);
        distortionMode2.setBounds (distortionModeArea);
        distortionMode3.setBounds (distortionModeArea);
        distortionMode4.setBounds (distortionModeArea);

        juce::Rectangle<int> graphArea = controlAreaMid.removeFromLeft (getWidth() / 7 * 2);

        // Graph Panel
        graphPanel.setBounds (graphArea);

        controlArea.removeFromLeft (getWidth() / 7 * 2);
        bandPanel.setBounds (controlArea);
        globalPanel.setBounds (controlAreaMid);

        juce::Rectangle<int> controlLeftKnobLeftArea = controlAreaMid.removeFromLeft (getWidth() / 7 * 2);
        juce::Rectangle<int> controlLeftKnobRightArea = controlLeftKnobLeftArea.removeFromRight (getWidth() / 7);

        controlLeftKnobLeftArea.removeFromTop (controlLeftKnobLeftArea.getHeight() / 4);
        controlLeftKnobLeftArea.removeFromBottom (controlLeftKnobLeftArea.getHeight() / 5);

        controlLeftKnobRightArea.removeFromTop (controlLeftKnobRightArea.getHeight() / 4);
        controlLeftKnobRightArea.removeFromBottom (controlLeftKnobRightArea.getHeight() / 5);
    }
    // Zoom button
    zoomButton.setBounds (getWidth() - 30, multiband.getY() + multiband.getHeight() - 30, getHeight() / 25, getHeight() / 25);

    // set look and feel scale
    otherLookAndFeel.setWindowScale(scale);
    bandPanel.setScale (scale);
    globalPanel.setScale (scale);
    multiband.setScale (scale);
}

void HayesDistortionAudioProcessorEditor::timerCallback()
{
    // bypassed
    if (processor.getBypassedState())
    {
        graphPanel.repaint();
        multiband.repaint();
    }
    else if (processor.isFFTBlockReady())
    {
        // not bypassed, repaint at the same time
        //(1<<11)
        // create a temp ddtData because sometimes pushNextSampleIntoFifo will replace the original
        // fftData after doingProcess and before painting.
        float tempFFTData[2 * 2048] = { 0 };
        memmove (tempFFTData, processor.getFFTData(), sizeof (tempFFTData));
        // doing process, fifo data to fft data
        processor.processFFT (tempFFTData);
        // prepare to paint the spectrum
        spectrum.prepareToPaintSpectrum (processor.getNumBins(), tempFFTData, processor.getSampleRate() / (float) processor.getFFTSize());

        graphPanel.repaint();
        spectrum.repaint();
        multiband.repaint();
        bandPanel.repaint();
        globalPanel.repaint();
        repaint();
    }
}

void HayesDistortionAudioProcessorEditor::sliderValueChanged (juce::Slider* slider)
{
}

void HayesDistortionAudioProcessorEditor::buttonClicked (juce::Button* clickedButton)
{
    if (clickedButton == &zoomButton)
    {
        if (zoomButton.getToggleState())
        {
            windowLeftButton.setVisible (true);
            windowRightButton.setVisible (true);
            if (windowLeftButton.getToggleState())
            {
                multiband.setVisible (true);
                bandPanel.setVisible (true);
                filterControl.setVisible (false);
                globalPanel.setVisible (false);
            }
            else
            {
                multiband.setVisible (false);
                bandPanel.setVisible (false);
                filterControl.setVisible (true);
                globalPanel.setVisible (true);
            }
            graphPanel.setVisible (true);
            zoomButton.setToggleState (false, juce::NotificationType::dontSendNotification);
        }
        else
        {
            windowLeftButton.setVisible (false);
            windowRightButton.setVisible (false);
            distortionMode1.setVisible (false);
            distortionMode2.setVisible (false);
            distortionMode3.setVisible (false);
            distortionMode4.setVisible (false);
            graphPanel.setVisible (false);
            bandPanel.setVisible (false);
            globalPanel.setVisible (false);
            zoomButton.setToggleState (true, juce::NotificationType::dontSendNotification);
        }
        resized();
    }
    if (clickedButton == &windowLeftButton)
    {
        if (windowLeftButton.getToggleState())
        {
            multiband.setVisible (true);
            bandPanel.setVisible (true);
            filterControl.setVisible (false);
            globalPanel.setVisible (false);
        }
        else
        {
            multiband.setVisible (false);
            bandPanel.setVisible (false);
            filterControl.setVisible (true);
            globalPanel.setVisible (true);
        }
    }
    for (int i = 0; i < 4; i++)
    {
        // bypass button for each band
        if (clickedButton == &multiband.getEnableButton (i))
        {
            bandPanel.setBandKnobsStates (i, clickedButton->getToggleState(), false);
        }
        // bypass button for band compressor/width
        if (clickedButton == &bandPanel.getCompButton (i) || clickedButton == &bandPanel.getWidthButton (i))
        {
            bool state = clickedButton->getToggleState();
            if (state)
            {
                multiband.setBandBypassStates (i, state);
                bandPanel.setBandKnobsStates (i, state, true);
            }
        }
    }
}

void HayesDistortionAudioProcessorEditor::comboBoxChanged (juce::ComboBox* combobox)
{
}

void HayesDistortionAudioProcessorEditor::setMenu (juce::ComboBox* combobox)
{
    addAndMakeVisible (combobox);

    combobox->addSectionHeading ("Soft Clipping");
    combobox->addItem ("Arctan", 1);
    combobox->addItem ("Exp", 2);
    combobox->addItem ("Tanh", 3);
    combobox->addItem ("Cubic", 4);
    combobox->addSeparator();

    combobox->addSectionHeading ("Hard Clipping");
    combobox->addItem ("Hard", 5);
    combobox->addItem ("Sausage", 6);
    combobox->addSeparator();

    combobox->addSectionHeading ("Foldback");
    combobox->addItem ("Sin", 7);
    combobox->addItem ("Linear", 8);
    combobox->addSeparator();

    combobox->addSectionHeading ("Other");
    combobox->addItem ("Limit", 9);
    combobox->addItem ("Single Sin", 10);
    combobox->addItem ("Logic", 11);
    combobox->addItem ("Pit", 12);
    combobox->addSeparator();

    combobox->setJustificationType (juce::Justification::centred);
    combobox->addListener (this);
}

void HayesDistortionAudioProcessorEditor::setLinearSlider (juce::Slider& slider)
{
    addAndMakeVisible (slider);
    slider.setSliderStyle (juce::Slider::LinearVertical);
    slider.setTextBoxStyle (juce::Slider::TextBoxAbove, false, TEXTBOX_WIDTH, TEXTBOX_HEIGHT);
}

void HayesDistortionAudioProcessorEditor::setDistortionGraph (juce::String modeId, juce::String driveId, juce::String recId, juce::String mixId, juce::String biasId, juce::String safeId)
{
    // paint distortion function
    int mode = static_cast<int> (*processor.apvts.getRawParameterValue (modeId));

    // protection
    float drive = static_cast<int> (*processor.apvts.getRawParameterValue (driveId));
    drive = drive * 6.5f / 100.0f;
    float powerDrive = powf (2, drive);

    float sampleMaxValue = processor.getSampleMaxValue (safeId);
    bool isSafeModeOn = *processor.apvts.getRawParameterValue (safeId);

    if (isSafeModeOn && sampleMaxValue * powerDrive > 2.0f)
    {
        drive = 2.0f / sampleMaxValue + 0.1 * std::log2f (powerDrive);
    }
    else
    {
        drive = powerDrive;
    }

    float rec = static_cast<float> (*processor.apvts.getRawParameterValue (recId));
    float mix = static_cast<float> (*processor.apvts.getRawParameterValue (mixId));
    float bias = static_cast<float> (*processor.apvts.getRawParameterValue (biasId));
    float rateDivide = static_cast<float> (*processor.apvts.getRawParameterValue (DOWNSAMPLE_ID));

    if (! *processor.apvts.getRawParameterValue (DOWNSAMPLE_BYPASS_ID))
        rateDivide = 1;

    graphPanel.setDistortionState (mode, rec, mix, bias, drive, rateDivide);
}

void HayesDistortionAudioProcessorEditor::setMultiband()
{
    multiband.sortLines();
    multiband.setLineRelatedBoundsByX();
    multiband.setSoloRelatedBounds();
}

void HayesDistortionAudioProcessorEditor::setFourComponentsVisibility (juce::Component& component1, juce::Component& component2, juce::Component& component3, juce::Component& component4, int bandNum)
{
    if (zoomButton.getToggleState())
    {
        component1.setVisible (false);
        component2.setVisible (false);
        component3.setVisible (false);
        component4.setVisible (false);
    }
    else
    {
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
}

void HayesDistortionAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    if (e.eventComponent == graphPanel.getOscilloscope())
        bandPanel.setSwitch (0, true);
    if (e.eventComponent == graphPanel.getDistortionGraph())
        bandPanel.setSwitch (1, true);
    if (e.eventComponent == graphPanel.getVuPanel())
        bandPanel.setSwitch (2, true);
    if (e.eventComponent == graphPanel.getWidthGraph())
        bandPanel.setSwitch (3, true);
    if (e.eventComponent == &multiband)
        updateWhenChangingFocus();
}

void HayesDistortionAudioProcessorEditor::updateWhenChangingFocus()
{
    focusIndex = multiband.getFocusIndex();    
    bool left = windowLeftButton.getToggleState();
    bool right = windowRightButton.getToggleState();
    if (left)
    { // if you select the left window, you will see audio wave and distortion function graphs.
        bandPanel.setFocusBandNum(focusIndex);
        graphPanel.setFocusBandNum(focusIndex);
    }
    else if (right)
    {
        graphPanel.setFocusBandNum(-1); // -1 means global
    }
    bandPanel.updateWhenChangingFocus();
    repaint();
}
