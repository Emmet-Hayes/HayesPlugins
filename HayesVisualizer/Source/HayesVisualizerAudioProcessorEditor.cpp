#include "HayesVisualizerAudioProcessor.h"
#include "HayesVisualizerAudioProcessorEditor.h"

HayesVisualizerAudioProcessorEditor::HayesVisualizerAudioProcessorEditor (HayesVisualizerAudioProcessor& p)
:   BaseAudioProcessorEditor (p)
,   processor (p)
{
    setLookAndFeel(&customLookAndFeel);
    openGLComponent = std::make_unique<OpenGLComponent>();
    addAndMakeVisible(openGLComponent.get());
    
    const auto ratio = static_cast<double> (defaultWidth) / defaultHeight;
    setResizable(false, true);
    getConstrainer()->setFixedAspectRatio (ratio);
    getConstrainer()->setSizeLimits(defaultWidth, defaultHeight, defaultWidth * 2, defaultHeight * 2);
    setSize (defaultWidth, defaultHeight);
}

HayesVisualizerAudioProcessorEditor::~HayesVisualizerAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void HayesVisualizerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void HayesVisualizerAudioProcessorEditor::resized()
{
    openGLComponent->setBounds(getLocalBounds());
}

void HayesVisualizerAudioProcessorEditor::timerCallback()
{
    openGLComponent->synthNoteColor = 4186.f;
    openGLComponent->audioAmplitude = 0.5f;
    repaint();
}
