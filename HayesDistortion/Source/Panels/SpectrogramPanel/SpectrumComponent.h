#pragma once

#include "../../GUI/DriveLookAndFeel.h"


class SpectrumComponent : public juce::Component
{
public:
    SpectrumComponent();

    void paint(juce::Graphics& g) override;
    void prepareToPaintSpectrum(int numberOfBins, float* spectrumData, float binWidth);
    static float transformToLog(double valueToTransform);
    static float transformFromLog(double between0and1);
    void resized() override;
    void paintSpectrum();

private:
    int numberOfBins;
    float spectrumData[1024] = { 0 };
    float maxData[1024] = { 0 };
    float maxDecibelValue{ -100.0f };
    float maxFreq { 0.0f };
    bool mouseOver { false };
    juce::Point<float> maxDecibelPoint;

    juce::Image currentSpectrumImage { juce::Image(juce::Image::ARGB, 1000, 300, true) };
    juce::Image maxSpectrumImage     { juce::Image(juce::Image::ARGB, 1000, 300, true) };

    static const int frequenciesForLines[];
    static const int numberOfLines;
    float mBinWidth;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpectrumComponent)
};
