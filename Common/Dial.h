#pragma once

#include <JuceHeader.h>


class Dial  : public juce::Slider
{
public:
    enum ColourIds
    {
        foregroundArcColourId,
        backgroundArcColourId,
        needleColourId,
        borderColourId,
    };

    explicit Dial (juce::RangedAudioParameter& param, juce::UndoManager* um = nullptr);

    void paint (juce::Graphics& g) override;
    void resized() override;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseDoubleClick (const juce::MouseEvent& e) override;

    bool keyPressed (const juce::KeyPress& k) override;

    void focusLost (FocusChangeType cause) override;

    float getValue() const;

    void setInterval (float newInterval);
    void setFineInterval (float newFineInterval);

    void setTextBoxColour (juce::Colour newColour);
    void setLabelColour (juce::Colour newColour);

    void setLabelText (const juce::String& newLabelText);

    void setAngle (float startAngleRadians, float endAngleRadians);

    std::function<void()> onValueChange;

private:
    void updateValue (float newValue);

    void drawDial (juce::Graphics& g);
    void drawBorder (juce::Graphics& g);

    juce::Label label;

    struct TextBox  : public juce::Label
    {
        juce::String valueShownWithEditor;

        TextBox();

        void resized() override;

        juce::TextEditor* createEditorComponent() override;

        void editorShown (juce::TextEditor* editor) override;
        void editorAboutToBeHidden (juce::TextEditor* editor) override;
    };

    TextBox textBox;

    float startAngle{};
    float endAngle{};

    float value{};

    juce::RangedAudioParameter& audioParam;
    juce::ParameterAttachment paramAttachment;

    float sensitivity { 0.01f };

    float interval { 1.0f };
    float fineInterval { 0.1f };

    int numDecPlaces { 2 };

    juce::Point<float> mousePosWhenLastDragged;

    juce::Rectangle<float> dialBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Dial)
};
