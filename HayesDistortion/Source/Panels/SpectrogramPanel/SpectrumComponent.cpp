#include "SpectrumComponent.h"


const int SpectrumComponent::frequenciesForLines[] = { 20, 30, 40, 50, 60, 70, 80, 90, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000, 20000 };
const int SpectrumComponent::numberOfLines = 28;

SpectrumComponent::SpectrumComponent() 
:   numberOfBins { 1024 }
,   mBinWidth { 44100 / (float)2048 }
{
}

void SpectrumComponent::paint(juce::Graphics& g)
{
    // paint background
    g.setColour (juce::Colours::darkmagenta.withBrightness(0.2f));
    g.fillAll();

    // paint horizontal lines and frequency numbers
    g.setColour (juce::Colours::lightgrey.withAlpha (0.2f));
    g.drawLine (0.f, static_cast<float>(getHeight() / 5), static_cast<float>(getWidth()), static_cast<float>(getHeight()) / 5, 1.f);

    for (int i = 0; i < numberOfLines; ++i)
    {
        const double proportion = frequenciesForLines[i] / 20000.0;
        int xPos = static_cast<int>(transformToLog (proportion * 20000) * (getWidth()));
        g.drawVerticalLine (xPos, static_cast<float>(getHeight() / 5), static_cast<float>(getHeight()));
        if (frequenciesForLines[i] == 10 || frequenciesForLines[i] == 100 || frequenciesForLines[i] == 200)
            g.drawFittedText (static_cast<juce::String> (frequenciesForLines[i]), xPos - 30, 0, 60, getHeight() / 5, juce::Justification::centred, 2);
        else if (frequenciesForLines[i] == 1000 || frequenciesForLines[i] == 10000 || frequenciesForLines[i] == 2000)
            g.drawFittedText (static_cast<juce::String> (frequenciesForLines[i] / 1000) + "k", xPos - 30, 0, 60, getHeight() / 5, juce::Justification::centred, 2);
        else if (frequenciesForLines[i] == 20)
            g.drawFittedText (static_cast<juce::String> (frequenciesForLines[i]), xPos - 30, 0, 60, getHeight() / 5, juce::Justification::right, 2);
        else if (frequenciesForLines[i] == 20000)
            g.drawFittedText (static_cast<juce::String> (frequenciesForLines[i] / 1000) + "k", xPos - 30, 0, 60, getHeight() / 5, juce::Justification::left, 2);
    }

    // paint current spectrum
    g.setColour (juce::Colours::white);
    paintSpectrum();
    currentSpectrumImage.multiplyAllAlphas (0.9f);
    currentSpectrumImage.moveImageSection (0, 10, 0, 0, currentSpectrumImage.getWidth(), currentSpectrumImage.getHeight());
    g.drawImageAt (currentSpectrumImage, 0, 0);

    // paint peak spectrum
    maxSpectrumImage.multiplyAllAlphas (0.5);
    g.drawImageAt (maxSpectrumImage, 0, 0);

    // paint peak text
    float mouseX = static_cast<float>(getMouseXYRelative().getX());
    float mouseY = static_cast<float>(getMouseXYRelative().getY());

    if (mouseX > 0 && mouseX < getWidth()
        && mouseY > 0 && mouseY < getHeight())
        mouseOver = true;
    else
        mouseOver = false;

    if (maxDecibelValue >= -99.9f && mouseOver)
    {
        int boxWidth = 100;
        int mdpx = static_cast<int>(maxDecibelPoint.getX());
        int mdpy = static_cast<int>(maxDecibelPoint.getY());
        g.setColour (juce::Colours::lightgrey);
        g.drawText(juce::String (maxDecibelValue, 1) + " db", mdpx - boxWidth / 2, 
                   mdpy - boxWidth / 4, boxWidth, boxWidth, juce::Justification::centred);
        g.drawText (juce::String (static_cast<int> (maxFreq)) + " Hz", mdpx - boxWidth / 2, 
                                  mdpy, boxWidth, boxWidth, juce::Justification::centred);
    }
    else
    {
        maxDecibelValue = -100.0f;
        maxFreq = 0.0f;
        maxDecibelPoint.setXY (-10.0f, -10.0f);
        for (int i = 0; i < 1024; i++)
            maxData[i] = 0;
    }
}

void SpectrumComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    currentSpectrumImage = currentSpectrumImage.rescaled (getWidth(), getHeight());
    maxSpectrumImage = maxSpectrumImage.rescaled (getWidth(), getHeight());
}

void SpectrumComponent::paintSpectrum()
{
    // this method is to paint spectrogram

    // init graphics
    juce::Graphics gCurrent (currentSpectrumImage);
    juce::Graphics gMax (maxSpectrumImage);

    auto width = getLocalBounds().getWidth();
    auto height = getLocalBounds().getHeight();
    auto mindB = -100.0f;
    auto maxdB = 0.0f;

    juce::Path currentSpecPath;
    currentSpecPath.startNewSubPath (0.f, static_cast<float>(height));

    juce::Path maxSpecPath;
    maxSpecPath.startNewSubPath (0.f, static_cast<float>(height + 1));
    int resolution = 2;
    for (int i = 1; i < numberOfBins; i += resolution)
    {
        // sample range [0, 1] to decibel range[-∞, 0] to [0, 1]
        float currentDecibel = juce::Decibels::gainToDecibels (spectrumData[i] / static_cast<float> (numberOfBins));
        float yPercent = juce::jmap (juce::jlimit (mindB, maxdB, currentDecibel),
                                     mindB,
                                     maxdB,
                                     0.0f,
                                     1.0f);

        // connect points
        double currentFreq = i * static_cast<double>(mBinWidth);
        float currentX = transformToLog (currentFreq) * width;
        float currentY = juce::jmap (yPercent, 0.0f, 1.0f, (float) height, 0.0f);
        currentSpecPath.lineTo (currentX, currentY);

        if (currentDecibel > maxDecibelValue)
        {
            maxDecibelValue = currentDecibel;
            maxFreq = static_cast<float>(currentFreq);
            maxDecibelPoint.setXY (currentX, currentY);
        }
        if (spectrumData[i] > maxData[i])
            maxData[i] = spectrumData[i];

        // reference: https://docs.juce.com/master/tutorial_spectrum_analyser.html
    }

    // this step is to round the path
    juce::Path roundedCurrentPath = currentSpecPath.createPathWithRoundedCorners (10.0f);

    // draw the outline of the path
    roundedCurrentPath.lineTo (static_cast<float>(width), static_cast<float>(height));
    roundedCurrentPath.lineTo (0.f, static_cast<float>(height));
    roundedCurrentPath.closeSubPath();

    juce::Path roundedMaxPath = maxSpecPath.createPathWithRoundedCorners (10.0f);
    roundedMaxPath.lineTo (static_cast<float>(width), static_cast<float>(height + 1));
    //    roundedMaxPath.lineTo(0, height);
    //    roundedMaxPath.closeSubPath();

    gCurrent.setColour (juce::Colours::aquamarine);

    juce::ColourGradient grad (juce::Colours::pink.withAlpha(0.8f), 0, 0, juce::Colours::red.withAlpha(0.8f), 0, static_cast<float>(getLocalBounds().getHeight()), false);

    gCurrent.setGradientFill (grad);
    gCurrent.fillPath (roundedCurrentPath);
    //    g.strokePath(roundedPath, juce::PathStrokeType(2));

    if (mouseOver)
    {
        gMax.setColour (juce::Colours::white);
        gMax.strokePath (roundedMaxPath, juce::PathStrokeType (2));
        gMax.drawEllipse (maxDecibelPoint.getX() - 2.0f, maxDecibelPoint.getY() + 10.0f, 4.0f, 4.0f, 1.0f);
    }
}

void SpectrumComponent::prepareToPaintSpectrum (int numBins, float* data, float binWidth)
{
    numberOfBins = numBins;
    memmove (spectrumData, data, sizeof (spectrumData));
    mBinWidth = binWidth;
}

float SpectrumComponent::transformToLog (double valueToTransform) // freq to x
{
    // input: 20-20000
    // output: x
    auto value = juce::mapFromLog10 (valueToTransform, 20.0, 20000.0);
    return static_cast<float> (value);
}

float SpectrumComponent::transformFromLog (double between0and1) // x to freq
{
    // input: 0.1-0.9 x pos
    // output: freq
    auto value = juce::mapToLog10 (between0and1, 20.0, 20000.0);
    return static_cast<float> (value);
}
