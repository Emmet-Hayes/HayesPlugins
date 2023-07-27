#pragma once

#include <JuceHeader.h>

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


class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();
    void drawCornerResizer (juce::Graphics& g, int w, int h, bool isMouseOver, bool isMouseDragging) override;
    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu, const juce::String& text,
                           const juce::String& shortcutKeyText, const juce::Drawable* icon,
                           const juce::Colour* textColourToUse) override;
    void drawLabel(juce::Graphics& g, juce::Label& label) override;
    juce::Label* createSliderTextBox(juce::Slider& slider) override;
    juce::Font getLabelFont(juce::Label& label) override;
    void drawComboBox(juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
	juce::Font getPopupMenuFont() override;
	void drawPopupMenuBackground(juce::Graphics&, int, int) override;
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, const float,
		                  const float, juce::Slider&) override;
    void setWindowScale(const float& newScale) { windowScale = newScale; }

protected:
    float windowScale { 1.0f };

private:
	juce::Font getCommonMenuFont(float);
    float rotaryOutlineBrightness { 1.0f };
};


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
