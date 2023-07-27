#pragma once

#include <JuceHeader.h>

#include "../../../HayesDistortionAudioProcessor.h"
#include "DistortionGraph.h"
#include "Oscilloscope.h"
#include "VUPanel.h"
#include "WidthGraph.h"


class GraphPanel : public juce::Component
{
public:
    GraphPanel (HayesDistortionAudioProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;
    void setDistortionState (int mode, float rec, float mix, float bias, float drive, float rateDivide);
    void setFocusBandNum (int num);

    Oscilloscope* getOscilloscope();
    VUPanel* getVuPanel();
    DistortionGraph* getDistortionGraph();
    WidthGraph* getWidthGraph();

private:
    HayesDistortionAudioProcessor& processor;

    Oscilloscope oscilloscope { processor };
    VUPanel vuPanel { processor };
    DistortionGraph distortionGraph { processor };
    WidthGraph widthGraph { processor };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphPanel)
};
