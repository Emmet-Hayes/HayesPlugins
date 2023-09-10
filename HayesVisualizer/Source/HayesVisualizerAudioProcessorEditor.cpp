#include "HayesVisualizerAudioProcessor.h"
#include "HayesVisualizerAudioProcessorEditor.h"


HayesVisualizerAudioProcessorEditor::HayesVisualizerAudioProcessorEditor (HayesVisualizerAudioProcessor& p)
:   BaseAudioProcessorEditor (p)
,   processor (p)
{
    setLookAndFeel(&customLookAndFeel);
    openGLComponent = std::make_unique<OpenGLComponent>();
    addAndMakeVisible(openGLComponent.get());
    
    errorComponent.setText("A version of OpenGL capable of rendering the visualizer shaders was not found. Sorry!", 
                           juce::dontSendNotification);
    addAndMakeVisible(&errorComponent); // displays error message on failed openGL load
    
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
}

void HayesVisualizerAudioProcessorEditor::timerCallback()
{
    openGLComponent->synthNoteColor = 4186.f;
    openGLComponent->audioAmplitude = 0.5f;
    
    isOpenGLAvailable = juce::OpenGLShaderProgram::getLanguageVersion() >= 4.2;
    if (isOpenGLAvailable && tryInitializeOpenGL == 0)
        openGLComponent->setBounds(getLocalBounds());
    else
        errorComponent.setBounds(0, getHeight() / 3, getWidth(), getHeight() / 3);
        
    tryInitializeOpenGL++; // don't try to init more than once
    repaint();
}
