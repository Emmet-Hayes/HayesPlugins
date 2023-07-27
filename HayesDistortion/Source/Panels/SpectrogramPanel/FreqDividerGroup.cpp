#include "FreqDividerGroup.h"


FreqDividerGroup::FreqDividerGroup (HayesDistortionAudioProcessor& p, int index) : processor (p), freqTextLabel (verticalLine)
{
    margin = getHeight() / 20.0f;

    addAndMakeVisible (verticalLine);

    verticalLine.addListener (this);

    if (index == 0)
    {
        lineStatelId = LINE_STATE_ID1;
        sliderFreqId = FREQ_ID1;
    }
    if (index == 1)
    {
        lineStatelId = LINE_STATE_ID2;
        sliderFreqId = FREQ_ID2;
    }
    if (index == 2)
    {
        lineStatelId = LINE_STATE_ID3;
        sliderFreqId = FREQ_ID3;
    }

    multiFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processor.apvts, sliderFreqId, verticalLine);

    addAndMakeVisible (freqTextLabel);
    // The parent component WON'T respond to mouse clicks,
    // while child components WILL respond to mouse clicks!
    setInterceptsMouseClicks (false, true);
    freqTextLabel.setLookAndFeel (&otherLookAndFeel);
}

FreqDividerGroup::~FreqDividerGroup()
{
    freqTextLabel.setLookAndFeel (nullptr);
}

void FreqDividerGroup::paint (juce::Graphics& /*g*/)
{
    if (getToggleState())
    {
        if (verticalLine.isMouseOverOrDragging() || freqTextLabel.isMouseOverCustom())
        {
            freqTextLabel.setFade (true, true);
            freqTextLabel.setVisible (true);
        }
        else
        {
            freqTextLabel.setFade (true, false);
        }
    }
    else
    {
        freqTextLabel.setFreq (-1);
        verticalLine.setXPercent (0);
    }
}

void FreqDividerGroup::resized()
{
    margin = getHeight() / 20.0f;
    size = getWidth() / 100.0f * 15;
    width = verticalLine.getWidth() / 2.0f;

    int imargin = static_cast<int>(margin);
    int isize = static_cast<int>(margin);
    int iwidth = static_cast<int>(margin);
    verticalLine.setBounds (0, 0, static_cast<int>(getWidth() / 10.0f), getHeight());
    freqTextLabel.setBounds (iwidth + imargin * 2, getHeight() / 5 + imargin, isize * 5, isize * 2);
}

void FreqDividerGroup::setDeleteState (bool deleteState)
{
    verticalLine.setDeleteState (deleteState);
}

void FreqDividerGroup::moveToX (int lineNum, float newXPercent, float pmargin, std::unique_ptr<FreqDividerGroup> freqDividerGroup[])
{
    if (!getToggleState())
        return;
    float leftLimit;
    float rightLimit;

    leftLimit = (verticalLine.getIndex() + 1) * pmargin;
    rightLimit = 1 - (lineNum - verticalLine.getIndex()) * pmargin;

    if (newXPercent < leftLimit)
    {
        newXPercent = leftLimit;
    }
    else if (newXPercent > rightLimit)
    {
        newXPercent = rightLimit;
    }

    verticalLine.setXPercent(newXPercent);
    verticalLine.setValue (SpectrumComponent::transformFromLog (newXPercent)); // * (44100 / 2.0)
    
    if (verticalLine.getLeft() >= 0 && freqDividerGroup[verticalLine.getLeft()]->getToggleState() && newXPercent - freqDividerGroup[verticalLine.getLeft()]->verticalLine.getXPercent() - pmargin < -0.00001f) // float is not accurate!!!!
    {
        freqDividerGroup[verticalLine.getLeft()]->moveToX (lineNum, newXPercent - pmargin, pmargin, freqDividerGroup);
    }
    if (verticalLine.getRight() > 0 && verticalLine.getRight() < lineNum && freqDividerGroup[verticalLine.getRight()]->getToggleState() && freqDividerGroup[verticalLine.getRight()]->verticalLine.getXPercent() - newXPercent - pmargin < -0.00001f)
    {
        freqDividerGroup[verticalLine.getRight()]->moveToX (lineNum, newXPercent + pmargin, pmargin, freqDividerGroup);
    }
}

VerticalLine& FreqDividerGroup::getVerticalLine()
{
    return verticalLine;
}

void FreqDividerGroup::buttonClicked (juce::Button* /*button*/)
{
}

void FreqDividerGroup::clicked (const juce::ModifierKeys& /*modifiers*/)
{
    // called by changing toggle state
    if (getToggleState())
        setVisible (true);
    else
        setVisible (false);
}

void FreqDividerGroup::sliderValueChanged (juce::Slider* slider)
{
    // TODO: maybe i don't need this
    // ableton move sliders
    if (slider == &verticalLine && getToggleState())
    {
        //dragLinesByFreq(freqDividerGroup[0].getValue(), getSortedIndex(0));
        double freq = slider->getValue();
        freqTextLabel.setFreq (static_cast<int>(freq));
        float xPercent = static_cast<float>(SpectrumComponent::transformToLog(freq));
        verticalLine.setXPercent(xPercent); // set freq -> set X percent
    }
}

void FreqDividerGroup::mouseDoubleClick (const juce::MouseEvent& /*e*/)
{
    // do nothing, override the silder function, which will reset value.
}

void FreqDividerGroup::setFreq (float f)
{
    verticalLine.setValue(static_cast<double>(f));
    verticalLine.setXPercent(static_cast<float>(SpectrumComponent::transformToLog (f)));
    freqTextLabel.setFreq(static_cast<int>(f));
}

int FreqDividerGroup::getFreq()
{
    return static_cast<int>(verticalLine.getValue());
}
void FreqDividerGroup::mouseUp (const juce::MouseEvent&) {}
void FreqDividerGroup::mouseEnter (const juce::MouseEvent&) {}
void FreqDividerGroup::mouseExit (const juce::MouseEvent&) {}
void FreqDividerGroup::mouseDown (const juce::MouseEvent&) {}
void FreqDividerGroup::mouseDrag (const juce::MouseEvent&) {}

void FreqDividerGroup::setScale (float scale)
{
    otherLookAndFeel.setWindowScale(scale);
    freqTextLabel.setScale (scale);
}
