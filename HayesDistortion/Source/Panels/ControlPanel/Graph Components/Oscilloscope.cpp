#include "Oscilloscope.h"
#include <JuceHeader.h>


Oscilloscope::Oscilloscope (HayesDistortionAudioProcessor& p) : processor (p)
{
    startTimerHz (60);
}

void Oscilloscope::paint (juce::Graphics& g)
{
    // draw outline
    g.setColour(COLOUR6);
    g.drawRect(getLocalBounds(), 1);

    juce::ColourGradient grad(juce::Colours::pink.withBrightness(0.9f), 
                              getWidth() / 2.0f, getHeight() / 2.0f, 
                              juce::Colours::red.withAlpha(0.0f), 
                              0.0f, getHeight(), true);
    g.setGradientFill(grad);

    bool monoChannel = false;
    historyL = processor.getHistoryArrayL();
    if (processor.getTotalNumInputChannels() == 1)
        monoChannel = true;
    else
        historyR = processor.getHistoryArrayR();

    juce::Path pathL;
    juce::Path pathR;

    float amp;
    if (monoChannel)
        amp = getHeight() / 2.0f;
    else
        amp = getHeight() / 4.0f;

    // get max
    float maxValue = 0.0f;

    for (int i = 0; i < historyL.size(); i++)
        if (historyL[i] > maxValue || historyR[i] > maxValue)
            maxValue = historyL[i] > historyR[i] ? historyL[i] : historyR[i];

    //TODO: this may cause high CPU usage! maybe use i += 2?
    float valL { 0 };
    float valR { 0 };

    for (int i = 0; i < getWidth(); i++)
    {
        int scaledIndex = static_cast<int>(i * (float) historyL.size() / (float) getWidth());

        valL = historyL[scaledIndex];
        if (!monoChannel)
            valR = historyR[scaledIndex];

        // normalize
        if (maxValue > 0.005f)
        {
            valL = valL / maxValue * 0.6f;
            if (!monoChannel)
                valR = valR / maxValue * 0.6f;
        }

        valL = juce::jlimit<float>(-1, 1, valL);
        if (!monoChannel)
            valR = juce::jlimit<float>(-1, 1, valR);

        if (i == 0)
        {
            pathL.startNewSubPath(0.0f, amp);
            if (!monoChannel)
                pathR.startNewSubPath(0.0f, amp * 3.0f);
        }
        else
        {
            pathL.lineTo(static_cast<float>(i), amp - valL * amp);
            if (!monoChannel)
                pathR.lineTo(static_cast<float>(i), amp * 3 - valR * amp);
        }
    }

    g.strokePath(pathL, juce::PathStrokeType (2.0));
    if (!monoChannel)
        g.strokePath(pathR, juce::PathStrokeType (2.0));

    // set color when mouse on
    if (isMouseOn && ! mZoomState)
    {
        g.setColour(juce::Colours::hotpink.withAlpha(0.05f));
        g.fillAll();
    }
}

void Oscilloscope::timerCallback()
{
    repaint();
}
