#pragma once
#include "../../Common/CentredLabel.h"

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

private:
	juce::Font getCommonMenuFont(float);
	
    float windowScale = 1.0f;
    float rotaryOutlineBrightness = 1.0f;
};
