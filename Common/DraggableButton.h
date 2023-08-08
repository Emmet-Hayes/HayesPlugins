#pragma once

#include <JuceHeader.h>


class DraggableButton : public juce::Component
{
public:
    DraggableButton() {}

    void paint(juce::Graphics&) override;
    void resized() override;

    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void setState(const bool state);

private:
    juce::Colour getColour();

    bool isEntered { false };
    bool mState { true };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DraggableButton)
};
