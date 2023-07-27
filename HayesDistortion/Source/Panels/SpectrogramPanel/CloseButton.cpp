#include "CloseButton.h"


void CloseButton::paint (juce::Graphics& g)
{
    if (isEntered)
    {
        g.setColour (COLOUR1);
        g.drawLine (getWidth() / 4.f, getHeight() / 4.f, getWidth() / 4.f * 3.f, getHeight() / 4.f * 3.f, 2.f);
        g.drawLine (getWidth() / 4.f, getHeight() / 4.f * 3.f, getWidth() / 4.f * 3.f, getHeight() / 4.f, 2.f);
    }
    else
    {
        g.setColour (COLOUR1.withBrightness (0.4f));
        g.drawLine (getWidth() / 4.f, getHeight() / 4.f, getWidth() / 4.f * 3.f, getHeight() / 4.f * 3.f, 2.f);
        g.drawLine (getWidth() / 4.f, getHeight() / 4.f * 3.f, getWidth() / 4.f * 3.f, getHeight() / 4.f, 2.f);
    }
}

void CloseButton::resized()
{
}

void CloseButton::mouseDown (const juce::MouseEvent& /*e*/)
{
    setToggleState (false, juce::sendNotification);
}

void CloseButton::mouseEnter (const juce::MouseEvent& /*e*/)
{
    isEntered = true;
}

void CloseButton::mouseExit (const juce::MouseEvent& /*e*/)
{
    isEntered = false;
}
