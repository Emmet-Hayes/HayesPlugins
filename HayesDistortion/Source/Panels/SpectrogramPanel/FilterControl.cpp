#include "../../GUI/InterfaceDefines.h"
#include "FilterControl.h"


FilterControl::FilterControl (HayesDistortionAudioProcessor& p, GlobalPanel& panel) 
:   processor (p)
,   globalPanel (panel)
{
    const auto& params = processor.getParameters();
    for (auto param : params)
        param->addListener (this);
    
    updateChain();
    startTimerHz (60);
    
    addAndMakeVisible (draggableLowButton);
    addAndMakeVisible (draggablePeakButton);
    addAndMakeVisible (draggableHighButton);
}

FilterControl::~FilterControl()
{
    const auto& params = processor.getParameters();
    for (auto param : params)
        param->removeListener (this);
}

void FilterControl::paint (juce::Graphics& g)
{
    if (isVisible())
        processor.setHistoryArray (5); // 5 means global
    
    bool isFilterEnabled = *processor.apvts.getRawParameterValue (FILTER_BYPASS_ID);
    if (isFilterEnabled)
    {
        draggableLowButton.setState (true);
        draggablePeakButton.setState (true);
        draggableHighButton.setState (true);
        
        g.setColour (juce::Colours::hotpink.withBrightness (0.8f));
        g.strokePath (responseCurve, juce::PathStrokeType (2.0f));
        
        g.setColour (juce::Colours::hotpink.withBrightness (0.8f).withAlpha (0.2f));
        g.fillPath (responseCurve);
    }
    else
    {
        draggableLowButton.setState (false);
        draggablePeakButton.setState (false);
        draggableHighButton.setState (false);
        
        g.setColour (juce::Colours::dimgrey.withAlpha (0.8f));
        g.strokePath (responseCurve, juce::PathStrokeType (2.0f));
        
        g.setColour (juce::Colours::dimgrey.withAlpha (0.8f).withAlpha (0.2f));
        g.fillPath (responseCurve);
    }
    
    int size = static_cast<int>(getWidth() / 1000.0f * 15);
    
    int buttonX = static_cast<int>(getMouseXYRelative().getX() - size / 2.0f);
    int buttonY = static_cast<int>(getMouseXYRelative().getY() - size / 2.0f);
    if (buttonX < 0)
        buttonX = 0;
    if (buttonX > getWidth())
        buttonX = getWidth() - size;
    if (buttonY < getHeight() / 48.0f * (24 - 15) - size / 2.0f)
        buttonY = static_cast<int>(getHeight() / 48.0f * (24 - 15) - size / 2.0f);
    if (buttonY > getHeight() / 48.0f * (24 + 15) - size / 2.0f)
        buttonY = static_cast<int>(getHeight() / 48.0f * (24 + 15) - size / 2.0f);
    
    if (draggableLowButton.isMouseButtonDown() && isFilterEnabled)
    {
        globalPanel.setToggleButtonState ("lowcut");
        draggableLowButton.setBounds (buttonX, buttonY, size, size);
        globalPanel.getLowcutFreqKnob().setValue (juce::mapToLog10 (static_cast<double> (getMouseXYRelative().getX() / static_cast<double> (getWidth())), 20.0, 20000.0));
        globalPanel.getLowcutGainKnob().setValue (15.0f * (getHeight() / 2.0f - getMouseXYRelative().getY()) / (getHeight() / 48.0f * 15.0f));
    }
    if (draggablePeakButton.isMouseButtonDown() && isFilterEnabled)
    {
        globalPanel.setToggleButtonState ("peak");
        draggablePeakButton.setBounds (buttonX, buttonY, size, size);
        globalPanel.getPeakFreqKnob().setValue (juce::mapToLog10 (static_cast<double> (getMouseXYRelative().getX() / static_cast<double> (getWidth())), 20.0, 20000.0));
        globalPanel.getPeakGainKnob().setValue (15.0f * (getHeight() / 2.0f - getMouseXYRelative().getY()) / (getHeight() / 48.0f * 15.0f));
    }
    if (draggableHighButton.isMouseButtonDown() && isFilterEnabled)
    {
        globalPanel.setToggleButtonState ("highcut");
        draggableHighButton.setBounds (buttonX, buttonY, size, size);
        globalPanel.getHighcutFreqKnob().setValue (juce::mapToLog10 (static_cast<double> (getMouseXYRelative().getX() / static_cast<double> (getWidth())), 20.0, 20000.0));
        globalPanel.getHighcutGainKnob().setValue (15.0f * (getHeight() / 2.0f - getMouseXYRelative().getY()) / (getHeight() / 48.0f * 15.0f));
    }
}

void FilterControl::resized()
{
    updateResponseCurve();
    setDraggableButtonBounds();
}

void FilterControl::setDraggableButtonBounds()
{
    int size = static_cast<int>(getWidth() / 1000.0f * 15);
    int lowcutX = static_cast<int>(getWidth() * juce::mapFromLog10(static_cast<double>(*processor.apvts.getRawParameterValue (LOWCUT_FREQ_ID)), 20.0, 20000.0));
    int peakX = static_cast<int>(getWidth() * juce::mapFromLog10(static_cast<double>(*processor.apvts.getRawParameterValue (PEAK_FREQ_ID)), 20.0, 20000.0));
    int highcutX = static_cast<int>(getWidth() * juce::mapFromLog10(static_cast<double>(*processor.apvts.getRawParameterValue (HIGHCUT_FREQ_ID)), 20.0, 20000.0));
    
    int lowcutY = static_cast<int>(getHeight() / 2.0f * *processor.apvts.getRawParameterValue(LOWCUT_GAIN_ID) / 24.0f);
    int peakY = static_cast<int>(getHeight() / 2.0f * *processor.apvts.getRawParameterValue(PEAK_GAIN_ID) / 24.0f);
    int highcutY = static_cast<int>(getHeight() / 2.0f * *processor.apvts.getRawParameterValue(HIGHCUT_GAIN_ID) / 24.0f);
    
    draggableLowButton.setBounds(lowcutX - size / 2, getHeight() / 2 - lowcutY - size / 2, size, size);
    draggablePeakButton.setBounds(peakX - size / 2, getHeight() / 2 - peakY - size / 2, size, size);
    draggableHighButton.setBounds(highcutX - size / 2, getHeight() / 2 - highcutY - size / 2, size, size);
}

void FilterControl::updateChain()
{
    auto chainSettings = getChainSettings (processor.apvts);
    
    monoChain.setBypassed<ChainPositions::LowCut>(chainSettings.lowCutBypassed);
    monoChain.setBypassed<ChainPositions::Peak>(chainSettings.peakBypassed);
    monoChain.setBypassed<ChainPositions::HighCut>(chainSettings.highCutBypassed);
    monoChain.setBypassed<ChainPositions::LowCutQ>(chainSettings.lowCutBypassed);
    monoChain.setBypassed<ChainPositions::HighCutQ>(chainSettings.highCutBypassed);
    
    auto peakCoefficients = makePeakFilter(chainSettings, processor.getSampleRate());
    updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
    
    auto lowCutCoefficients = makeLowCutFilter(chainSettings, processor.getSampleRate());
    auto highCutCoefficients = makeHighCutFilter(chainSettings, processor.getSampleRate());
    
    auto lowCutQCoefficients = makeLowcutQFilter(chainSettings, processor.getSampleRate());
    updateCoefficients(monoChain.get<ChainPositions::LowCutQ>().coefficients, lowCutQCoefficients);
    auto highCutQCoefficients = makeHighcutQFilter(chainSettings, processor.getSampleRate());
    updateCoefficients(monoChain.get<ChainPositions::HighCutQ>().coefficients, highCutQCoefficients);
    
    updateCutFilter(monoChain.get<ChainPositions::LowCut>(),
                     lowCutCoefficients,
                     chainSettings.lowCutSlope);
    
    updateCutFilter(monoChain.get<ChainPositions::HighCut>(),
                     highCutCoefficients,
                     chainSettings.highCutSlope);
}

void FilterControl::updateResponseCurve()
{
    int w = getWidth();
    
    auto& lowcut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highcut = monoChain.get<ChainPositions::HighCut>();
    auto& lowcutQ = monoChain.get<ChainPositions::LowCutQ>();
    auto& highcutQ = monoChain.get<ChainPositions::HighCutQ>();
    
    auto sampleRate = processor.getSampleRate();
    
    std::vector<float> mags;
    mags.resize (w);
    
    for (int i = 0; i < w; ++i)
    {
        double mag = 1.0f;
        auto freq = juce::mapToLog10 (double (i) / double (w), 20.0, 20000.0);
        
        if (! monoChain.isBypassed<ChainPositions::Peak>())
            mag *= peak.coefficients->getMagnitudeForFrequency (freq, sampleRate);
        
        if (! monoChain.isBypassed<ChainPositions::LowCut>())
        {
            if (! lowcut.isBypassed<0>())
                mag *= lowcut.get<0>().coefficients->getMagnitudeForFrequency (freq, sampleRate);
            if (! lowcut.isBypassed<1>())
                mag *= lowcut.get<1>().coefficients->getMagnitudeForFrequency (freq, sampleRate);
            if (! lowcut.isBypassed<2>())
                mag *= lowcut.get<2>().coefficients->getMagnitudeForFrequency (freq, sampleRate);
            if (! lowcut.isBypassed<3>())
                mag *= lowcut.get<3>().coefficients->getMagnitudeForFrequency (freq, sampleRate);
            mag *= lowcutQ.coefficients->getMagnitudeForFrequency (freq, sampleRate);
        }
        
        if (! monoChain.isBypassed<ChainPositions::HighCut>())
        {
            if (! highcut.isBypassed<0>())
                mag *= highcut.get<0>().coefficients->getMagnitudeForFrequency (freq, sampleRate);
            if (! highcut.isBypassed<1>())
                mag *= highcut.get<1>().coefficients->getMagnitudeForFrequency (freq, sampleRate);
            if (! highcut.isBypassed<2>())
                mag *= highcut.get<2>().coefficients->getMagnitudeForFrequency (freq, sampleRate);
            if (! highcut.isBypassed<3>())
                mag *= highcut.get<3>().coefficients->getMagnitudeForFrequency (freq, sampleRate);
            mag *= highcutQ.coefficients->getMagnitudeForFrequency (freq, sampleRate);
        }
        
        mags[i] = juce::Decibels::gainToDecibels(mag);
    }
    
    responseCurve.clear();
    
    const double outputMin = getHeight();
    const double outputMax = 0;
    auto map = [outputMin, outputMax] (double input)
    {
        return juce::jmap (input, -24.0, 24.0, outputMin, outputMax);
    };
    
    juce::Point<float> startPoint (-3.f, static_cast<float>(getHeight() + 2));
    juce::Point<float> endPoint (static_cast<float>(getWidth() + 3), static_cast<float>(getHeight() + 2));
    
    responseCurve.startNewSubPath (startPoint);
    
    for (size_t i = 0; i < mags.size(); ++i)
        responseCurve.lineTo (0 + i, map (mags[i]));
    
    responseCurve.lineTo (endPoint);
    
    responseCurve.closeSubPath();
}

void FilterControl::parameterValueChanged (int /*parameterIndex*/, float /*newValue*/)
{
    parametersChanged.set (true);
}

void FilterControl::timerCallback()
{
    if (parametersChanged.compareAndSetBool (false, true))
    {
        updateChain();
        updateResponseCurve();
        setDraggableButtonBounds();
    }
    repaint();
}
