#include "../../../GUI/InterfaceDefines.h"
#include "../../../Utility/AudioHelpers.h"
#include "VUMeter.h"


VUMeter::VUMeter(HayesDistortionAudioProcessor* inProcessor)
:   mProcessor(inProcessor),
    mIsInput(true),
    mCh0Level(0),
    mCh1Level(0)
{
    setInterceptsMouseClicks(false, false);
}

VUMeter::~VUMeter()
{
}

void VUMeter::paint (juce::Graphics& g)
{
    
    g.setColour(COLOUR6);
    const int meterWidth = getWidth() / 3;
    
    if (mProcessor->getTotalNumInputChannels() == 2)
    {
        // left
        g.fillRect(0, 0, meterWidth, getHeight());
        // right
        g.fillRect(meterWidth * 2, 0, meterWidth, getHeight());
    }
    else
    {
        g.fillRect(meterWidth, 0, meterWidth, getHeight());
    }
    
    int ch0fill = static_cast<int>(getHeight() - (getHeight() * mCh0Level));
    int ch1fill = static_cast<int>(getHeight() - (getHeight() * mCh1Level));
    
    if (ch0fill < 0)
        ch0fill = 0;
    if (ch1fill < 0)
        ch1fill = 0;
    
    g.setColour(juce::Colours::magenta.withBrightness(0.9f));
    
    if (mProcessor->getTotalNumInputChannels() == 2)
    {
        g.fillRect(0, ch0fill, meterWidth, getHeight());
        g.fillRect(meterWidth * 2, ch1fill, meterWidth, getHeight());
    }
    else
    {
        g.fillRect(meterWidth, ch0fill, meterWidth, getHeight());
    }
    
}

void VUMeter::resized()
{
}

void VUMeter::setParameters(bool isInput, juce::String meterName)
{
    mIsInput = isInput;
    mMeterName = meterName;
    startTimerHz(30);
}

void VUMeter::timerCallback()
{
    float updatedCh0Level = 0.0f;
    float updatedCh1Level = 0.0f;

    if(mIsInput) // input
    {
        updatedCh0Level = mProcessor->getInputMeterRMSLevel(0, mMeterName);
        updatedCh1Level = mProcessor->getInputMeterRMSLevel(1, mMeterName);
    }
    else // output
    {
        updatedCh0Level = mProcessor->getOutputMeterRMSLevel(0, mMeterName);
        updatedCh1Level = mProcessor->getOutputMeterRMSLevel(1, mMeterName);
    }
    
    if (updatedCh0Level > mCh0Level)
    {
        mCh0Level = updatedCh0Level;
    }
    else
    {
        mCh0Level = static_cast<float>(SMOOTH_COEFF * (mCh0Level - updatedCh0Level) + updatedCh0Level);
    }
    
    if (updatedCh1Level > mCh1Level)
    {
        mCh1Level = updatedCh1Level;
    }
    else
    {
        mCh1Level = static_cast<float>(SMOOTH_COEFF * (mCh1Level - updatedCh1Level) + updatedCh1Level);
    }
    
    mCh0Level = helper_denormalize(mCh0Level);
    mCh1Level = helper_denormalize(mCh1Level);
    
    // repaint if not equal to 0
    if (mCh0Level && mCh1Level)
    {
        repaint();
    }
}

float VUMeter::getLeftChannelLevel()
{
    return mCh0Level;
}

float VUMeter::getRightChannelLevel()
{
    return mCh1Level;
}
