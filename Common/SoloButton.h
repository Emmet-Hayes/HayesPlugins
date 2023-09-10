#pragma once

#include "CustomLookAndFeel.h"


class SoloButton : public juce::ToggleButton
{
public:
    SoloButton() {}

    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseEnter (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;

private:
    bool isEntered = false;
    juce::Colour getColour();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoloButton)
};
