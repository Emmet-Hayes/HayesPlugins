#pragma once

#include <JuceHeader.h>

#include "../../GUI/DriveLookAndFeel.h"


class EnableButton : public juce::ToggleButton
{
public:
    EnableButton() {}
    
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void mouseEnter (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;
    
private:
    juce::Colour getColour();

    bool isEntered = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnableButton)
};
