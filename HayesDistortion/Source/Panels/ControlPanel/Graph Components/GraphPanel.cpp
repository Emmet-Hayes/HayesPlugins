#include "GraphPanel.h"
#include <JuceHeader.h>

GraphPanel::GraphPanel (HayesDistortionAudioProcessor& p) : processor (p)
{
    addAndMakeVisible (oscilloscope);
    addAndMakeVisible (vuPanel);
    addAndMakeVisible (distortionGraph);
    addAndMakeVisible (widthGraph);
    setInterceptsMouseClicks (false, true);
}

GraphPanel::~GraphPanel()
{
}

void GraphPanel::paint (juce::Graphics& g)
{
    if (oscilloscope.isVisible())
    {
        if (! oscilloscope.getZoomState())
        {
            oscilloscope.setBounds (0, 0, getWidth() / 2, getHeight() / 2);
            vuPanel.setVisible (true);
            distortionGraph.setVisible (true);
            widthGraph.setVisible (true);
        }
        else
        {
            // zoom in
            oscilloscope.setBounds (0, 0, getWidth(), getHeight());
            vuPanel.setVisible (false);
            distortionGraph.setVisible (false);
            widthGraph.setVisible (false);
        }
    }

    if (distortionGraph.isVisible())
    {
        if (! distortionGraph.getZoomState())
        {
            // zoom out
            distortionGraph.setBounds (getWidth() / 2, 0, getWidth() / 2, getHeight() / 2);
            oscilloscope.setVisible (true);
            vuPanel.setVisible (true);
            widthGraph.setVisible (true);
        }
        else
        {
            // zoom in
            distortionGraph.setBounds (0, 0, getWidth(), getHeight());
            oscilloscope.setVisible (false);
            vuPanel.setVisible (false);
            widthGraph.setVisible (false);
        }
    }

    if (vuPanel.isVisible())
    {
        if (! vuPanel.getZoomState())
        {
            // zoom out
            vuPanel.setBounds (0, getHeight() / 2, getWidth() / 2, getHeight() / 2);
            oscilloscope.setVisible (true);
            distortionGraph.setVisible (true);
            widthGraph.setVisible (true);
        }
        else
        {
            // zoom in
            vuPanel.setBounds (0, 0, getWidth(), getHeight());
            oscilloscope.setVisible (false);
            distortionGraph.setVisible (false);
            widthGraph.setVisible (false);
        }
    }

    if (widthGraph.isVisible())
    {
        if (! widthGraph.getZoomState())
        {
            // zoom out
            widthGraph.setBounds (getWidth() / 2, getHeight() / 2, getWidth() / 2, getHeight() / 2);
            oscilloscope.setVisible (true);
            distortionGraph.setVisible (true);
            vuPanel.setVisible (true);
        }
        else
        {
            // zoom in
            widthGraph.setBounds (0, 0, getWidth(), getHeight());
            oscilloscope.setVisible (false);
            distortionGraph.setVisible (false);
            vuPanel.setVisible (false);
        }
    }
}

void GraphPanel::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
}

void GraphPanel::setDistortionState (int mode, float rec, float mix, float bias, float drive, float rateDivide)
{
    distortionGraph.setState (mode, rec, mix, bias, drive, rateDivide);
}

Oscilloscope* GraphPanel::getOscilloscope()
{
    return &oscilloscope;
}

VUPanel* GraphPanel::getVuPanel()
{
    return &vuPanel;
}

DistortionGraph* GraphPanel::getDistortionGraph()
{
    return &distortionGraph;
}

WidthGraph* GraphPanel::getWidthGraph()
{
    return &widthGraph;
}

void GraphPanel::setFocusBandNum (int num)
{
    vuPanel.setFocusBandNum (num);
}
