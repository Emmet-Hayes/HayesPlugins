#include "GraphTemplate.h"


void GraphTemplate::paint (juce::Graphics& g)
{
    g.setColour (COLOUR6);
    g.drawRect (getLocalBounds(), 1);
}

void GraphTemplate::resized()
{
}

void GraphTemplate::setScale (float pscale)
{
    scale = pscale;
}

float GraphTemplate::getScale()
{
    return scale;
}

bool GraphTemplate::getZoomState()
{
    return mZoomState;
}

void GraphTemplate::setZoomState (bool zoomState)
{
    mZoomState = zoomState;
}

void GraphTemplate::mouseDown (const juce::MouseEvent& /*e*/)
{
    if (mZoomState)
    {
        mZoomState = false;
        scale = 1;
    }
    else
    {
        mZoomState = true;
        scale = 2;
    }
}

void GraphTemplate::mouseEnter (const juce::MouseEvent& /*e*/)
{
    isMouseOn = true;
}

void GraphTemplate::mouseExit (const juce::MouseEvent& /*e*/)
{
    isMouseOn = false;
}
