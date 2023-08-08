#include "DraggableButton.h"


void DraggableButton::paint(juce::Graphics& g)
{
    g.setColour (getColour().darker().darker());
    g.fillEllipse (0, 0, static_cast<float>(getWidth()), static_cast<float>(getHeight()));

    g.setColour (getColour());
    g.fillEllipse (getWidth() / 5.0f, getHeight() / 5.0f, getWidth() / 5.0f * 3, getHeight() / 5.0f * 3);
}

void DraggableButton::resized()
{
}

void DraggableButton::mouseEnter(const juce::MouseEvent& /*e*/)
{
    isEntered = true;
}

void DraggableButton::mouseExit(const juce::MouseEvent& /*e*/)
{
    isEntered = false;
}

juce::Colour DraggableButton::getColour()
{
    if (mState && isEntered)
        return juce::Colours::hotpink.brighter();
    else if (mState && ! isEntered)
        return juce::Colours::hotpink;

    return juce::Colours::dimgrey;
}

void DraggableButton::setState(const bool state)
{
    mState = state;
}
