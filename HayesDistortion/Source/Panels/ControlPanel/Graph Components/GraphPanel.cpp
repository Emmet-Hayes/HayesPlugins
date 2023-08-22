#include "GraphPanel.h"


GraphPanel::GraphPanel (HayesDistortionAudioProcessor& p) : processor (p)
{
    addAndMakeVisible (oscilloscope);
    addAndMakeVisible (distortionGraph);
    setInterceptsMouseClicks (false, true);
}

void GraphPanel::paint (juce::Graphics& /*g*/)
{
    if (oscilloscope.isVisible())
    {
        if (! oscilloscope.getZoomState())
        {
            oscilloscope.setBounds(0, 0, getWidth() / 2, getHeight());
            distortionGraph.setVisible(true);
        }
        else
        {
            oscilloscope.setBounds(0, 0, getWidth(), getHeight());
            distortionGraph.setVisible(false);
        }
    }

    if (distortionGraph.isVisible())
    {
        if (! distortionGraph.getZoomState())
        {
            distortionGraph.setBounds(getWidth() / 2, 0, getWidth() / 2, getHeight());
            oscilloscope.setVisible(true);
        }
        else
        {
            distortionGraph.setBounds(0, 0, getWidth(), getHeight());
            oscilloscope.setVisible(false);
        }
    }
}

void GraphPanel::resized()
{
}

void GraphPanel::setDistortionState (int mode, float rec, float mix, float bias, float drive, float rateDivide)
{
    distortionGraph.setState(mode, rec, mix, bias, drive, rateDivide);
}

Oscilloscope* GraphPanel::getOscilloscope()
{
    return &oscilloscope;
}

DistortionGraph* GraphPanel::getDistortionGraph()
{
    return &distortionGraph;
}

void GraphPanel::setFocusBandNum (int num)
{
    //vuPanel.setFocusBandNum (num);
}
