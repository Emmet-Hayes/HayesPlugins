#pragma once

#include "CustomLookAndFeel.h"
#include "VerticalLine.h"


class CloseButton : public juce::ToggleButton
{
public:
    CloseButton() {}

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

    bool isEntered { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CloseButton)
};
