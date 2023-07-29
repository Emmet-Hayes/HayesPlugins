#pragma once

#include <JuceHeader.h>

#include "../../GUI/DriveLookAndFeel.h"


class VerticalLine : public juce::Slider
{
public:
    VerticalLine() {}

    void paint (juce::Graphics&) override;
    void resized() override;

    void setDeleteState (bool deleteState);
    void setXPercent (float x);
    float getXPercent();
    void setIndex (int index);
    int getIndex();
    int getLeft();
    int getRight();
    void moveToX (int lineNum, float newXPercent, float margin, std::unique_ptr<VerticalLine> verticalLines[]);

private:
    bool isEntered = false;

    void mouseUp (const juce::MouseEvent& e) override;
    void mouseEnter (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDoubleClick (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;

    bool mDeleteState = false;
    float xPercent {  0.0f };
    int leftIndex  { -1 }; // left index
    int rightIndex { -1 }; // right index
    int index      { -1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VerticalLine)
};
