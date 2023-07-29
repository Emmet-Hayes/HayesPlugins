#pragma once

#include <JuceHeader.h>

#include "../../GUI/DriveLookAndFeel.h"
#include "VerticalLine.h"
#include "SpectrumComponent.h"


class FreqTextLabel : public juce::Component
                    , juce::Timer
{
public:
    FreqTextLabel (VerticalLine& v);
    ~FreqTextLabel() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void setFreq (int freq);
    int getFreq();
    void setScale (float scale);
    bool isMouseOverCustom();
    void timerCallback() override;
    void setFade (bool update, bool isFadeIn);

private:
    VerticalLine& verticalLine;
    int mFrequency;
    float mScale = 1.0f;
    bool mUpdate = false;
    bool mFadeIn = true;
    int currentStep = 0;
    const int maxStep = 5;

    juce::Label freqLabel;
    CustomLookAndFeel flatButtonLnf;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FreqTextLabel)
};
