#include "../../../Common/CentredLabel.h"

#include "DriveLookAndFeel.h"


DriveLookAndFeel::DriveLookAndFeel()
{
    setColour(juce::Slider::textBoxTextColourId, KNOB_SUBFONT_COLOUR);
    setColour(juce::Slider::textBoxBackgroundColourId, COLOUR6);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentWhite);
    setColour(juce::Slider::textBoxHighlightColourId, COLOUR7);
    setColour(juce::Label::textColourId, COLOUR_FONT);
    setColour(juce::Label::textWhenEditingColourId, COLOUR1);
    setColour(juce::Label::outlineColourId, juce::Colours::transparentWhite);
    setColour(juce::Label::backgroundWhenEditingColourId, COLOUR6);
    setColour(juce::Label::outlineWhenEditingColourId, juce::Colours::transparentWhite);
    setColour(juce::Slider::trackColourId, COLOUR1);
    setColour(juce::Slider::thumbColourId, COLOUR5);
    setColour(juce::Slider::backgroundColourId, COLOUR6);
}

void DriveLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
    const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    auto outline = COLOUR6;
    auto fill = COLOUR1;

    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10);

    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = radius * 0.2f;
    auto arcRadius = radius - lineW * 0.5f;

    juce::Path backgroundArc;
    backgroundArc.addCentredArc(bounds.getCentreX(),
        bounds.getCentreY(),
        arcRadius,
        arcRadius,
        0.0f,
        rotaryStartAngle,
        rotaryEndAngle,
        true);

    g.setColour(outline);
    g.strokePath(backgroundArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path backgroundShadowArc;
    backgroundShadowArc.addCentredArc(bounds.getCentreX(),
        bounds.getCentreY(),
        arcRadius + lineW / 2.0f,
        arcRadius + lineW / 2.0f,
        0.0f,
        0,
        static_cast<float>(2 * M_PI),
        true);

    backgroundShadowArc.addCentredArc(bounds.getCentreX(),
        bounds.getCentreY(),
        arcRadius - lineW / 2.0f,
        arcRadius - lineW / 2.0f,
        0.0f,
        0,
        static_cast<float>(2 * M_PI),
        true);
    drawInnerShadow(g, backgroundShadowArc);
    float reductAngle = toAngle - (1.0f - reductionPrecent) * 2 * static_cast<float>(M_PI);

    if (reductAngle < rotaryStartAngle)
        reductAngle = rotaryStartAngle;

    if (slider.isEnabled())
    {
        juce::Path valueArc;
        valueArc.addCentredArc(bounds.getCentreX(),
            bounds.getCentreY(),
            arcRadius,
            arcRadius,
            0.0f,
            rotaryStartAngle,
            reductAngle,
            true);

        juce::Path circlePath;
        circlePath.addCentredArc(bounds.getCentreX(),
            bounds.getCentreY(),
            arcRadius,
            arcRadius,
            0.0f,
            0,
            static_cast<float>(2 * M_PI),
            true);

        // make the color loop move
        if (sampleMaxValue > 0.00001f)
        {
            if (changePos < circlePath.getLength())
            {
                changePos += 10;
            }
            else
            {
                changePos = 0;
            }
        }
        else
        {
        }
        
        juce::Point<float> p1 = circlePath.getPointAlongPath(changePos);
        x1 = p1.x;
        y1 = p1.y;
        x2 = p1.x + 100 * scale;
        y2 = p1.y + 100 * scale;

        // draw real drive path
        juce::ColourGradient grad(juce::Colours::red, x1, y1,
            COLOUR1, x2, y2, true);
        g.setGradientFill(grad);

        g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        if (reductionPrecent != 1)
        {
            // draw reducted path
            juce::Path valueArcReduce;
            valueArcReduce.addCentredArc(bounds.getCentreX(),
                bounds.getCentreY(),
                arcRadius,
                arcRadius,
                0.0f,
                reductAngle - 0.02f,
                toAngle,
                true);

            juce::ColourGradient grad2(juce::Colours::red.withAlpha(0.5f), x1, y1,
                COLOUR1.withAlpha(0.5f), x2, y2, true);
            g.setGradientFill(grad2);

            g.strokePath(valueArcReduce, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    }

    // draw inner circle
    float diameterInner = juce::jmin(width, height) * 0.4f;
    float radiusInner = diameterInner / 2;
    float centerX = static_cast<float>(x + width / 2);
    float centerY = static_cast<float>(y + height / 2);
    float rx = centerX - radiusInner;
    float ry = centerY - radiusInner;
    float angle = rotaryStartAngle + (sliderPos * (rotaryEndAngle - rotaryStartAngle));

    juce::Rectangle<float> dialArea(rx, ry, diameterInner, diameterInner);

    // draw big circle
    juce::Path dialTick;
    dialTick.addRectangle(0.0f, -radiusInner, radiusInner * 0.1f, radiusInner * 0.3f);
    juce::ColourGradient grad(juce::Colours::black, centerX, centerY,
        juce::Colours::white.withBrightness(slider.isEnabled() ? 0.9f : 0.5f), radiusInner, radiusInner, true);
    g.setGradientFill(grad);
    g.fillEllipse(dialArea);

    // draw small circle
    juce::Rectangle<float> smallDialArea(rx + radiusInner / 10.0f * 3, ry + radiusInner / 10.0f * 3, diameterInner / 10.0f * 7, diameterInner / 10.0f * 7);

    juce::Colour innerColor = KNOB_INNER_COLOUR;

    if (slider.isMouseOverOrDragging() && slider.isEnabled())
        innerColor = innerColor.brighter();

    g.setColour(innerColor);
    g.fillEllipse(smallDialArea);

    // draw colorful inner circle
    if (sampleMaxValue > 0.00001f && slider.isEnabled())
    {
        // draw colorful tick
        g.setColour(juce::Colour(255, static_cast<int>(juce::jmax(255 - sampleMaxValue * 2000, 0.0f)), 0));
        g.fillPath(dialTick, juce::AffineTransform::rotation(angle).translated(centerX, centerY));
    }
    else // draw grey 2 layers circle
    {
        // draw grey tick
        g.setColour(juce::Colours::lightgrey.withBrightness(slider.isEnabled() ? 0.5f : 0.2f));
        g.fillPath(dialTick, juce::AffineTransform::rotation(angle).translated(centerX, centerY));
    }

}

// resize slider and textbox size
juce::Slider::SliderLayout DriveLookAndFeel::getSliderLayout(juce::Slider& slider)
{
    // 1. compute the actually visible textBox size from the slider textBox size and some additional constraints
    int minXSpace = 0;
    int minYSpace = 0;

    auto textBoxPos = slider.getTextBoxPosition();

    if (textBoxPos == juce::Slider::TextBoxLeft || textBoxPos == juce::Slider::TextBoxRight)
        minXSpace = 30;
    else
        minYSpace = 15;

    auto localBounds = slider.getLocalBounds();

    auto textBoxWidth = juce::jmax(0, juce::jmin(static_cast<int>(slider.getTextBoxWidth() * scale), localBounds.getWidth() - minXSpace));
    auto textBoxHeight = juce::jmax(0, juce::jmin(static_cast<int>(slider.getTextBoxHeight() * scale), localBounds.getHeight() - minYSpace));

    juce::Slider::SliderLayout layout;

    // 2. set the textBox bounds
    if (textBoxPos != juce::Slider::NoTextBox)
    {
        if (slider.isBar())
        {
            layout.textBoxBounds = localBounds;
        }
        else
        {
            layout.textBoxBounds.setWidth(textBoxWidth);
            layout.textBoxBounds.setHeight(textBoxHeight);

            if (textBoxPos == juce::Slider::TextBoxLeft)
                layout.textBoxBounds.setX(0);
            else if (textBoxPos == juce::Slider::TextBoxRight)
                layout.textBoxBounds.setX(localBounds.getWidth() - textBoxWidth);
            else /* above or below -> centre horizontally */
                layout.textBoxBounds.setX((localBounds.getWidth() - textBoxWidth) / 2);

            if (textBoxPos == juce::Slider::TextBoxAbove)
                layout.textBoxBounds.setY(0);
            else if (textBoxPos == juce::Slider::TextBoxBelow)
                layout.textBoxBounds.setY(localBounds.getHeight() - textBoxHeight); // changed here!
            else                                                                         /* left or right -> centre vertically */
                layout.textBoxBounds.setY((localBounds.getHeight() - textBoxHeight) / 2);
        }
    }

    // 3. set the slider bounds
    layout.sliderBounds = localBounds;

    if (slider.isBar())
    {
        layout.sliderBounds.reduce(1, 1); // bar border
    }
    else
    {
        if (textBoxPos == juce::Slider::TextBoxLeft)
            layout.sliderBounds.removeFromLeft(textBoxWidth);
        else if (textBoxPos == juce::Slider::TextBoxRight)
            layout.sliderBounds.removeFromRight(textBoxWidth);
        else if (textBoxPos == juce::Slider::TextBoxAbove)
            layout.sliderBounds.removeFromTop(textBoxHeight);
        else if (textBoxPos == juce::Slider::TextBoxBelow)
            layout.sliderBounds.removeFromBottom(textBoxHeight);

        const int thumbIndent = getSliderThumbRadius(slider);

        if (slider.isHorizontal())
            layout.sliderBounds.reduce(thumbIndent, 0);
        else if (slider.isVertical())
            layout.sliderBounds.reduce(0, thumbIndent);
    }

    return layout;
}

void DriveLookAndFeel::fillTextEditorBackground(juce::Graphics& g, int width, int height, juce::TextEditor& textEditor)
{
    float cornerSize = 10.0f * scale;
    if (dynamic_cast<juce::AlertWindow*> (textEditor.getParentComponent()) != nullptr)
    {
        g.setColour(textEditor.findColour(juce::TextEditor::backgroundColourId));
        g.fillRoundedRectangle(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), cornerSize);
        g.setColour(textEditor.findColour(juce::TextEditor::outlineColourId));
        g.drawHorizontalLine(height - 1, 0.0f, static_cast<float> (width));
    }
    else
    {
        g.setColour(textEditor.findColour(juce::TextEditor::backgroundColourId));
        g.fillRoundedRectangle(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), cornerSize);
    }
}

void DriveLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    float cornerSize = 10.0f * scale;
    g.setColour(label.findColour(juce::Label::backgroundColourId));
    g.fillRoundedRectangle(label.getLocalBounds().toFloat(), cornerSize);

    if (!label.isBeingEdited())
    {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const juce::Font font(getLabelFont(label));

        g.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(alpha));
        g.setFont(font);

        auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());

        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
            juce::jmax(1, (int)((float)textArea.getHeight() / font.getHeight())),
            label.getMinimumHorizontalScale());

        g.setColour(label.findColour(juce::Label::outlineColourId).withMultipliedAlpha(alpha));
    }
    else if (label.isEnabled())
    {
        g.setColour(label.findColour(juce::Label::outlineColourId));
    }

    g.drawRoundedRectangle(label.getLocalBounds().toFloat(), cornerSize, 1);
}

juce::Font DriveLookAndFeel::getTextButtonFont(juce::TextButton&, int /*buttonHeight*/)
{
    return juce::Font(KNOB_FONT, "Regular", KNOB_FONT_SIZE * scale);
}

juce::Font DriveLookAndFeel::getComboBoxFont(juce::ComboBox& /*box*/)
{
    return juce::Font(KNOB_FONT, "Regular", KNOB_FONT_SIZE * scale);
}

juce::Font DriveLookAndFeel::getPopupMenuFont()
{
    return juce::Font(KNOB_FONT, "Regular", KNOB_FONT_SIZE * scale);
}

juce::Font DriveLookAndFeel::getLabelFont(juce::Label& /*label*/)
{
    return juce::Font(KNOB_FONT, "Regular", KNOB_FONT_SIZE * scale);
    ;
}
