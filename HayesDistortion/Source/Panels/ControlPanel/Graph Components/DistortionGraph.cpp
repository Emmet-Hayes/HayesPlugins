#include "DistortionGraph.h"


DistortionGraph::DistortionGraph (HayesDistortionAudioProcessor& p) 
:   processor (p)
{
    const auto& params = processor.getParameters();
    for (auto param : params)
        param->addListener (this);

    startTimerHz (60);
}

DistortionGraph::~DistortionGraph()
{
    const auto& params = processor.getParameters();
    for (auto param : params)
        param->removeListener (this);
}

void DistortionGraph::paint (juce::Graphics& g)
{
    auto frameRight = getLocalBounds();

    // draw outline
    g.setColour (COLOUR6);
    g.drawRect (getLocalBounds(), 1);

    // symmetrical distortion
    if (mode <= 15)
    {
        float functionValue = 0.0f;
        float mixValue;

        const int numPix = frameRight.getWidth(); // you might experiment here, if you want less steps to speed up

        float driveScale = 1.0f;
        float maxValue = 2.0f * driveScale * mix + 2.0f * (1.0f - mix);
        float value = -maxValue; // minimum (leftmost)  value for your graph
        float valInc = (maxValue - value) / numPix;
        float xPos = static_cast<float>(frameRight.getX());
        const float posInc = static_cast<float>(frameRight.getWidth() / numPix);

        juce::Path p;

        bool edgePointL = false;
        bool edgePointR = false;

        float step = 0.5f; // step = 0.5f, maybe high cpu but more accurate graph

        for (float i = 1; i < numPix; i += step)
        {
            value += valInc * step;
            xPos += posInc * step;

            float valueAfterDrive = value;

            // downsample
            if (rateDivide > 1.0f)
                valueAfterDrive = ceilf (valueAfterDrive / (rateDivide / 256.0f)) * (rateDivide / 256.0f);

            valueAfterDrive = valueAfterDrive * drive + bias;

            switch (mode)
            {
                case 0:  functionValue = waveshaping::arctanSoftClipping (valueAfterDrive); break;
                case 1:  functionValue = waveshaping::expSoftClipping    (valueAfterDrive); break;
                case 2:  functionValue = waveshaping::tanhSoftClipping   (valueAfterDrive); break;
                case 3:  functionValue = waveshaping::cubicSoftClipping  (valueAfterDrive); break;
                case 4:  functionValue = waveshaping::hardClipping       (valueAfterDrive); break;
                case 5:  functionValue = waveshaping::sausageFattener    (valueAfterDrive); break;
                case 6:  functionValue = waveshaping::sinFoldback        (valueAfterDrive); break;
                case 7:  functionValue = waveshaping::linFoldback        (valueAfterDrive); break;
                case 8:  functionValue = waveshaping::limitClip          (valueAfterDrive); break;
                case 9:  functionValue = waveshaping::singleSinClip      (valueAfterDrive); break;
                case 10: functionValue = waveshaping::logicClip          (valueAfterDrive); break;
                case 11: functionValue = waveshaping::tanclip            (valueAfterDrive); break;
            }

            // retification
            functionValue = waveshaping::rectificationProcess (functionValue, rec);

            // mix
            functionValue = (1.0f - mix) * value + mix * functionValue;
            mixValue = (2.0f / 3.0f) * functionValue;
            float yPos = frameRight.getCentreY() - frameRight.getHeight() * mixValue / 2.0f;

            // draw points
            if (yPos < frameRight.getY())
            {
                if (edgePointR == false)
                {
                    yPos = static_cast<float>(frameRight.getY());
                    edgePointR = true;
                }
                else
                    continue;
            }

            if (yPos > frameRight.getBottom())
            {
                if (edgePointL == false)
                    continue;
                else
                    yPos = static_cast<float>(frameRight.getBottom());
            }
            else if (edgePointL == false)
            {
                if (mode == 0)
                {
                    p.startNewSubPath (xPos, static_cast<float>(frameRight.getBottom()));
                    p.lineTo (xPos, yPos);
                }
                else
                    p.startNewSubPath (xPos, yPos);

                edgePointL = true;
            }
            p.lineTo (xPos, yPos);
        }

        juce::ColourGradient grad (juce::Colours::violet.withBrightness (0.9f), frameRight.getX() + frameRight.getWidth() / 2, frameRight.getY() + frameRight.getHeight() / 2, juce::Colours::red.withBrightness (0.9f).withAlpha (0.0f), frameRight.getX(), static_cast<float>(frameRight.getY() + frameRight.getHeight() / 2), true);
        g.setGradientFill (grad);
        g.strokePath (p, juce::PathStrokeType (2.0f));
    }

    if (isMouseOn && ! mZoomState)
    {
        g.setColour (juce::Colours::yellow.withAlpha (0.05f));
        g.fillAll();
    }
}

void DistortionGraph::setState (int pmode, float prec, float pmix, float pbias, float pdrive, float prateDivide)
{
    mode = pmode;
    rec = prec;
    mix = pmix;
    bias = pbias;
    drive = pdrive;
    rateDivide = prateDivide;
}

void DistortionGraph::parameterValueChanged (int /*parameterIndex*/, float /*newValue*/)
{
    parametersChanged.set (true);
}

void DistortionGraph::timerCallback()
{
    if (parametersChanged.compareAndSetBool (false, true)) {}
    repaint();
}
