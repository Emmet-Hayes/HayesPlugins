#pragma once

#include "../../../HayesDistortionAudioProcessor.h"
#include "DistortionGraph.h"
#include "Oscilloscope.h"


class GraphPanel : public juce::Component
{
public:
    GraphPanel (HayesDistortionAudioProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;
    void setDistortionState (int mode, float rec, float mix, float bias, float drive, float rateDivide);
    void setFocusBandNum (int num);

    Oscilloscope* getOscilloscope();
    DistortionGraph* getDistortionGraph();

private:
    HayesDistortionAudioProcessor& processor;

    Oscilloscope oscilloscope { processor };
    DistortionGraph distortionGraph { processor };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphPanel)
};
