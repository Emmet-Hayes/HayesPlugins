#pragma once
#include <JuceHeader.h>

class CentredLabel : public juce::Label
{
public:
    juce::TextEditor* createEditorComponent() override
    {
        juce::TextEditor* textEditor = juce::Label::createEditorComponent();
        textEditor->setJustification(juce::Justification::centred);
        return textEditor;
    }
};