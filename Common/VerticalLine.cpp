#include "VerticalLine.h"


void VerticalLine::paint (juce::Graphics& g)
{
    // draw line that will be added next
    g.setColour (juce::Colour(244, 63, 208));
    g.fillRect (getWidth() / 10.f * 4.f, 0.f, getWidth() / 10.f * 2.f, static_cast<float> (getHeight()));

    if (isMouseOverOrDragging())
    {
        g.setColour (juce::Colour(244, 63, 208).withAlpha (0.2f));
        g.fillAll();
    }
}

void VerticalLine::resized()
{
}

void VerticalLine::mouseUp (const juce::MouseEvent& /*e*/)
{
    //    move = false;
}

void VerticalLine::mouseDoubleClick (const juce::MouseEvent& /*e*/)
{
    // do nothing, override the silder function, which will reset value.
}

void VerticalLine::mouseEnter (const juce::MouseEvent& /*e*/)
{
    isEntered = true;
}

void VerticalLine::mouseExit (const juce::MouseEvent& /*e*/)
{
    isEntered = false;
}

void VerticalLine::mouseDrag (const juce::MouseEvent& /*e*/)
{
    // this will call multiband mouseDrag
}

void VerticalLine::mouseDown (const juce::MouseEvent& /*e*/)
{
}

void VerticalLine::setDeleteState (bool deleteState)
{
    mDeleteState = deleteState;
}

void VerticalLine::setXPercent (float x)
{
    xPercent = x;
}

float VerticalLine::getXPercent()
{
    return xPercent;
}

void VerticalLine::setIndex (int pindex)
{
    index = pindex;
}

int VerticalLine::getIndex()
{
    return index;
}

int VerticalLine::getLeft()
{
    return index - 1;
}

int VerticalLine::getRight()
{
    return index + 1;
}

void VerticalLine::moveToX (int lineNum, float newXPercent, float margin, std::unique_ptr<VerticalLine> verticalLines[])
{
    float leftLimit;
    float rightLimit;

    leftLimit = (index + 1) * margin;
    rightLimit = 1 - (lineNum - index) * margin;

    if (newXPercent < leftLimit)
        newXPercent = leftLimit;
    if (newXPercent > rightLimit)
        newXPercent = rightLimit;

    if (leftIndex >= 0 && newXPercent - verticalLines[leftIndex]->getXPercent() - margin < -0.00001f) // float is not accurate!!!!
    {
        verticalLines[leftIndex]->moveToX (lineNum, newXPercent - margin, margin, verticalLines);
    }
    if (rightIndex < lineNum && verticalLines[rightIndex]->getXPercent() - newXPercent - margin < -0.00001f)
    {
        verticalLines[rightIndex]->moveToX (lineNum, newXPercent + margin, margin, verticalLines);
    }
    xPercent = newXPercent;
}
