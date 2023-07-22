#include "CustomLookAndFeel.h"

CustomLookAndFeel::CustomLookAndFeel() 
{
    setColour(juce::Slider::thumbColourId, juce::Colours::darkmagenta);
    setColour(juce::ComboBox::textColourId, juce::Colours::white);
    setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::white);
}

void CustomLookAndFeel::drawCornerResizer (juce::Graphics& g, int w, int h, bool isMouseOver, bool isMouseDragging)
{
    juce::ignoreUnused (isMouseDragging);

    auto lineThickness = juce::jmin ((float) w, (float) h) * 0.07f;

    for (float i = 0.0f; i < 1.0f; i += 0.3f)
    {
        auto colour = isMouseOver ? juce::Colours::blue : juce::Colours::grey;
        g.setColour (colour);

        g.drawLine ((float) w * i,
                    (float) h + 1.0f,
                    (float) w + 1.0f,
                    (float) h * i,
                    lineThickness);

        g.drawLine ((float) w * i + lineThickness,
                    (float) h + 1.0f,
                    (float) w + 1.0f,
                    (float) h * i + lineThickness,
                    lineThickness);
    }
}

void CustomLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                       bool isSeparator, bool isActive, bool isHighlighted,
                       bool isTicked, bool hasSubMenu, const juce::String& text,
                       const juce::String& shortcutKeyText, const juce::Drawable* icon,
                       const juce::Colour* textColourToUse)
{
    if (isHighlighted)
        g.fillAll(juce::Colours::silver);

    g.setColour(isHighlighted ? juce::Colours::black : juce::Colours::white);
    g.setFont(juce::Font("Lucida Console", 13.0f * windowScale, juce::Font::bold));
    g.drawText(text, area, juce::Justification::centred);
}
    
void CustomLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.fillAll (label.findColour (juce::Label::backgroundColourId));
    
    if (! label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const juce::Font font (getLabelFont (label).withHeight(16.0f * windowScale));
        
        g.setColour (label.findColour (juce::Label::textColourId).withMultipliedAlpha (alpha));
        g.setFont (font);
        juce::Rectangle<int> textArea (label.getBorderSize().subtractedFrom (label.getLocalBounds()));
        g.drawFittedText (label.getText(), textArea, label.getJustificationType(),
                          juce::jmax (1, (int) (textArea.getHeight() / font.getHeight())),
                          label.getMinimumHorizontalScale());
        g.setColour (label.findColour (juce::Label::outlineColourId).withMultipliedAlpha (alpha));
    }
    else if (label.isEnabled())
    {
        g.setColour (label.findColour (juce::Label::outlineColourId));
    }

    g.drawRect (label.getLocalBounds());
}

juce::Label* CustomLookAndFeel::createSliderTextBox(juce::Slider& slider)
{
    auto* l = new CentredLabel();

    l->setColour(juce::Label::textColourId, slider.findColour(juce::Slider::textBoxTextColourId));
    l->setColour(juce::Label::backgroundColourId,
        (slider.getSliderStyle() == juce::Slider::LinearBar || slider.getSliderStyle() == juce::Slider::LinearBarVertical)
        ? juce::Colours::transparentBlack
        : slider.findColour(juce::Slider::textBoxBackgroundColourId));
    l->setColour(juce::Label::outlineColourId, slider.findColour(juce::Slider::textBoxOutlineColourId));
    l->setColour(juce::TextEditor::textColourId, slider.findColour(juce::Slider::textBoxTextColourId));
    l->setColour(juce::TextEditor::backgroundColourId,
        slider.findColour(juce::Slider::textBoxBackgroundColourId)
        .withAlpha((slider.isMouseOverOrDragging() && slider.getSliderStyle() != juce::Slider::SliderStyle::LinearBar
                    && slider.getSliderStyle() != juce::Slider::SliderStyle::LinearBarVertical) ? 0.7f : 0.3f));
    l->setColour(juce::TextEditor::outlineColourId, slider.findColour(juce::Slider::textBoxOutlineColourId));
    float fontSize = 11.0f * windowScale; // base font size
    float maxFontSize = fontSize; // maximum font size

    // Dynamically adjust font size based on label bounds
    juce::String testString = "0.000"; // change this to your worst case string
    float width = l->getFont().getStringWidth(testString);
    while (width > l->getBounds().getWidth() && fontSize > 1) {
        fontSize -= 0.5f;
        width = l->getFont().withHeight(fontSize).getStringWidth(testString);
    }

    // Don't exceed the maximum font size
    fontSize = juce::jmin(fontSize, maxFontSize);
    l->setFont(juce::Font("Lucida Console", fontSize, juce::Font::bold));
    l->setJustificationType(juce::Justification::centred);

    return l;
}

juce::Font CustomLookAndFeel::getLabelFont(juce::Label& label)
{
    return juce::Font(juce::Font("Lucida Console", 13.f * windowScale, juce::Font::bold));
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, 
                                         const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider) 
{
    double radius  = juce::jmin(width / 2, height / 2) - 4.0f;
    double centreX = x + width * 0.5f;
    double centreY = y + height * 0.5f;
    double rx = centreX - radius;
    double ry = centreY - radius;
    double rw = radius * 2.0f;
    double angle = static_cast<double>(rotaryStartAngle) + sliderPos * static_cast<double>(rotaryEndAngle - rotaryStartAngle);

    g.setColour(juce::Colours::whitesmoke);
    g.setOpacity(0.33f);
    g.fillEllipse(rx, ry, rw, rw);

    g.setColour(juce::Colours::bisque);
    g.setOpacity(1.0f);
    g.drawEllipse(rx, ry, rw, rw, 2.0f);

    juce::Path p;
    double pointerLength = radius * 0.5f;
    double pointerThickness = 5.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY)); //animate
    g.setColour(juce::Colours::seashell);
    g.fillPath(p);
}

void CustomLookAndFeel::drawComboBox(juce::Graphics& g, int /*w*/, int /*h*/, bool /*isDown*/, int /*bx*/, int /*by*/, int /*bw*/, int /*bh*/, juce::ComboBox& /*cb*/)
{
    juce::Colour c(30, 8, 33);
    g.setColour(c);
    g.fillAll();
}

juce::Font CustomLookAndFeel::getComboBoxFont(juce::ComboBox & c)
{
    return getCommonMenuFont(windowScale);
}

juce::Font CustomLookAndFeel::getPopupMenuFont() 
{
    return getCommonMenuFont(windowScale);
}

juce::Font CustomLookAndFeel::getCommonMenuFont(float s) 
{
    return juce::Font("Lucida Console", 11.f * s, juce::Font::bold);
}

void CustomLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height) 
{
    juce::Colour c(30, 8, 33);
    g.setColour(c);
    g.fillAll();
}
