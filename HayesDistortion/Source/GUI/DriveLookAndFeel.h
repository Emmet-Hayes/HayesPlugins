#pragma once

#include <JuceHeader.h>

#include "../../Common/CustomLookAndFeel.h"
#include "InterfaceDefines.h"


#ifndef JUCE_MAC
 constexpr double M_PI = 3.1415926;
#endif

inline void drawInnerShadow(juce::Graphics& g, juce::Path target) {
    // resets the Clip Region when the function returns
    juce::Graphics::ScopedSaveState saveState(g);

    // invert the path's fill shape and enlarge it,
    // so it casts a shadow
    juce::Path shadowPath(target);
    shadowPath.addRectangle(target.getBounds().expanded(10));
    shadowPath.setUsingNonZeroWinding(false);

    // reduce clip region to avoid the shadow
    // being drawn outside of the shape to cast the shadow on
    g.reduceClipRegion(target);

    juce::DropShadow ds(COLOUR7, 5, { 0, 1 });
    ds.drawForPath(g, shadowPath);
}


class DriveLookAndFeel : public CustomLookAndFeel
{
public:
    DriveLookAndFeel();
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
        const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) override;
    juce::Slider::SliderLayout getSliderLayout(juce::Slider& slider) override;
    void fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor) override;
    void drawLabel(juce::Graphics& g, juce::Label& label) override;
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    juce::Font getComboBoxFont(juce::ComboBox& /*box*/) override;
    juce::Font getPopupMenuFont() override;
    juce::Font getLabelFont(juce::Label& label) override;

    float scale { 1.0f };
    float reductionPrecent { 1.0f };
    float sampleMaxValue { 0.0f };
private:
    float x1 { 0 }, y1 { 0 }, x2 { 0 }, y2 { 0 };
    float changePos { 0 }; // use this to change drive knob gradient
};


class HighPassButtonLnf : public CustomLookAndFeel
{
public:
    void drawButtonBackground(juce::Graphics& g,
        juce::Button& button,
        const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
};


class LowPassButtonLnf : public CustomLookAndFeel
{
public:
    void drawButtonBackground(juce::Graphics& g,
        juce::Button& button,
        const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
};


class BandPassButtonLnf : public CustomLookAndFeel
{
public:
    void drawButtonBackground(juce::Graphics& g,
        juce::Button& button,
        const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;

    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
};
