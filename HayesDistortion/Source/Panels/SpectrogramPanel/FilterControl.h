#pragma once

#include <JuceHeader.h>

#include "../../HayesDistortionAudioProcessor.h"
#include "../../Utility/AudioHelpers.h"
#include "../ControlPanel/GlobalPanel.h"
#include "DraggableButton.h"


class FilterControl : public juce::Component
                    , juce::AudioProcessorParameter::Listener
                    , juce::Timer
{
public:
    FilterControl (HayesDistortionAudioProcessor&, GlobalPanel&);
    ~FilterControl() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int /*parameterIndex*/, bool /*gestureIsStarting*/) override {}
    void timerCallback() override;

private:
    HayesDistortionAudioProcessor& processor;
    GlobalPanel& globalPanel;
    juce::Path responseCurve;

    using Filter = juce::dsp::IIR::Filter<float>;
    using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;
    using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter, Filter, Filter>;
    MonoChain monoChain;

    juce::Atomic<bool> parametersChanged { false };
    void updateResponseCurve();
    void updateChain();
    void setDraggableButtonBounds();
    DraggableButton draggableLowButton, draggablePeakButton, draggableHighButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterControl)
};
