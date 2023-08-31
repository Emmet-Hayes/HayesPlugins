#include "SoloButton.h"


void SoloButton::paint (juce::Graphics& g)
{
    g.setColour (getColour().darker().darker());
    g.fillEllipse (0, 0, static_cast<float>(getWidth()), static_cast<float>(getHeight()));
    g.setColour (getColour());
    g.drawText ("S", 0, 0, getWidth(), getHeight(), juce::Justification::centred);
}

void SoloButton::resized()
{
}

void SoloButton::mouseEnter (const juce::MouseEvent& /*e*/)
{
    isEntered = true;
}

void SoloButton::mouseExit (const juce::MouseEvent& /*e*/)
{
    isEntered = false;
}

juce::Colour SoloButton::getColour()
{
    if (isEntered)
    {
        if (! getToggleState())
            return juce::Colours::grey.withAlpha (0.8f);
        else
            return juce::Colour(244, 63, 208).withAlpha (0.8f);
    }
    else
    {
        if (! getToggleState())
            return juce::Colours::grey;
        else
            return juce::Colour(244, 63, 208);
    }
}
