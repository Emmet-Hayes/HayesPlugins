#include "EnableButton.h"


void EnableButton::paint (juce::Graphics& g)
{
    g.setColour (getColour().darker().darker());
    g.fillEllipse (0.0f, 0.0f, static_cast<float>(getWidth()), static_cast<float>(getHeight()));

    g.setColour (getColour());
    auto bounds = juce::Rectangle<int> (getLocalBounds()).toFloat();
    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto lineW = radius * 0.2f;
    auto arcRadius = radius - lineW * 2.0f;

    juce::Path backgroundArc;
    backgroundArc.addCentredArc (bounds.getCentreX(),
                                 bounds.getCentreY(),
                                 arcRadius,
                                 arcRadius,
                                 0.0f,
                                 2 * static_cast<float>(M_PI) * 0.1f,
                                 2 * static_cast<float>(M_PI) * 0.9f,
                                 true);

    g.strokePath (backgroundArc, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.drawLine (getWidth() / 2.0f, getHeight() / 6.0f, getWidth() / 2.0f, getHeight() / 2.0f, lineW);
}

void EnableButton::resized()
{
}

void EnableButton::mouseEnter (const juce::MouseEvent& /*e*/)
{
    isEntered = true;
}

void EnableButton::mouseExit (const juce::MouseEvent& /*e*/)
{
    isEntered = false;
}

juce::Colour EnableButton::getColour()
{
    if (isEntered)
    {
        if (! getToggleState())
            return juce::Colours::grey.withAlpha (0.8f);
        else
            return COLOUR1.withAlpha (0.8f);
    }
    else
    {
        if (! getToggleState())
            return juce::Colours::grey;
        else
            return COLOUR1;
    }
}
