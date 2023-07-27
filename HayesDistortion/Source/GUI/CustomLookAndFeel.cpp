#include "CustomLookAndFeel.h"
#include "CentredLabel.h"


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
        auto font = getLabelFont (label);
        
        // Dynamically adjust font size based on label bounds
        juce::String labelText = label.getText();
        float width = font.getStringWidth(labelText);
        while (width > label.getWidth() && font.getHeight() > 1) {
            font.setHeight(font.getHeight() - 1.0f);
            width = font.getStringWidth(labelText);
        }

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

// customize knobs
void DriveLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
    const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider& slider)
{
    // draw outline
    auto outline = COLOUR6;
    auto fill = COLOUR1;

    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10);

    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    // auto lineW = jmin(8.0f, radius * 0.2f);
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

    // draw shadow
    juce::Path backgroundShadowArc;
    backgroundShadowArc.addCentredArc(bounds.getCentreX(),
        bounds.getCentreY(),
        arcRadius + lineW / 2.0f,
        arcRadius + lineW / 2.0f,
        0.0f,
        0,
        2 * M_PI,
        true);
    //backgroundShadowArc.lineTo(<#Point<float> end#>)
    backgroundShadowArc.addCentredArc(bounds.getCentreX(),
        bounds.getCentreY(),
        arcRadius - lineW / 2.0f,
        arcRadius - lineW / 2.0f,
        0.0f,
        0,
        2 * M_PI,
        true);
    drawInnerShadow(g, backgroundShadowArc);
    float reductAngle = toAngle - (1.0f - reductionPrecent) * 2 * M_PI;

    // safe reduce paint
    if (reductAngle < rotaryStartAngle)
    {
        reductAngle = rotaryStartAngle;
    }

    if (slider.isEnabled())
    {
        // real drive path
        juce::Path valueArc;
        valueArc.addCentredArc(bounds.getCentreX(),
            bounds.getCentreY(),
            arcRadius,
            arcRadius,
            0.0f,
            rotaryStartAngle,
            reductAngle,
            true);

        // circle path, is used to draw dynamic color loop
        juce::Path circlePath;
        circlePath.addCentredArc(bounds.getCentreX(),
            bounds.getCentreY(),
            arcRadius,
            arcRadius,
            0.0f,
            0,
            2 * M_PI,
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

            juce::ColourGradient grad(juce::Colours::red.withAlpha(0.5f), x1, y1,
                COLOUR1.withAlpha(0.5f), x2, y2, true);
            g.setGradientFill(grad);

            g.strokePath(valueArcReduce, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }
    }

    // draw inner circle
    float diameterInner = juce::jmin(width, height) * 0.4f;
    float radiusInner = diameterInner / 2;
    float centerX = x + width / 2;
    float centerY = y + height / 2;
    float rx = centerX - radiusInner;
    float ry = centerY - radiusInner;
    float angle = rotaryStartAngle + (sliderPos * (rotaryEndAngle - rotaryStartAngle));

    juce::Rectangle<float> dialArea(rx, ry, diameterInner, diameterInner);

    // draw big circle
    juce::Path dialTick;
    dialTick.addRectangle(0, -radiusInner, radiusInner * 0.1f, radiusInner * 0.3);
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
        g.setColour(juce::Colour(255, juce::jmax(255 - sampleMaxValue * 2000, 0.0f), 0));
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

juce::Font DriveLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    return juce::Font(KNOB_FONT, "Regular", KNOB_FONT_SIZE * scale);
}

// combobox customize font
juce::Font DriveLookAndFeel::getComboBoxFont(juce::ComboBox& /*box*/)
{
    return juce::Font(KNOB_FONT, "Regular", KNOB_FONT_SIZE * scale);
}

juce::Font DriveLookAndFeel::getPopupMenuFont()
{
    return juce::Font(KNOB_FONT, "Regular", KNOB_FONT_SIZE * scale);
}

// label customize font
juce::Font DriveLookAndFeel::getLabelFont(juce::Label& label)
{
    return juce::Font(KNOB_FONT, "Regular", KNOB_FONT_SIZE * scale);
    ;
}


void HighPassButtonLnf::drawButtonBackground(juce::Graphics& g,
    juce::Button& button,
    const juce::Colour& backgroundColour,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    auto cornerSize = 10.0f * windowScale;
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);

    auto baseColour = backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 1.0f)
        .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);

    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        baseColour = baseColour.contrasting(shouldDrawButtonAsDown ? 0.2f : 0.05f);

    g.setColour(baseColour);

    auto flatOnLeft = button.isConnectedOnLeft();
    auto flatOnRight = button.isConnectedOnRight();
    auto flatOnTop = button.isConnectedOnTop();
    auto flatOnBottom = button.isConnectedOnBottom();

    if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom)
    {
        juce::Path path;
        path.addRoundedRectangle(bounds.getX(), bounds.getY(),
            bounds.getWidth(), bounds.getHeight(),
            cornerSize, cornerSize,
            !(flatOnLeft || flatOnTop),
            !(flatOnRight || flatOnTop),
            !(flatOnLeft || flatOnBottom),
            !(flatOnRight || flatOnBottom));

        g.fillPath(path);

        g.setColour(button.findColour(juce::ComboBox::outlineColourId));
        g.strokePath(path, juce::PathStrokeType(1.0f));
    }
    else
    {
        g.fillRoundedRectangle(bounds, cornerSize);
        g.setColour(button.findColour(juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
    }

    // draw high pass filter
    juce::Colour textOnColour = button.findColour(juce::TextButton::textColourOnId);
    juce::Colour textOffColour = button.findColour(juce::TextButton::textColourOffId);
    juce::Colour textColour;

    if (button.getToggleState())
        textColour = textOnColour;
    else
        textColour = textOffColour;

    if (button.isEnabled()) g.setColour(textColour);
    else g.setColour(textColour.darker(0.5f));

    juce::Path p;
    float width = bounds.getWidth();
    float height = bounds.getHeight();
    p.startNewSubPath(width * 0.25f, height);
    p.lineTo(width / 2.0f, height / 2.0f);
    p.lineTo(width - 1, height / 2.0f);
    juce::Path roundedPath = p.createPathWithRoundedCorners(5.0f);
    g.strokePath(roundedPath, juce::PathStrokeType(1.0f));

    // fill above part
    if (button.isEnabled()) g.setColour(textColour.withAlpha(0.5f));
    else g.setColour(textColour.withAlpha(0.5f).darker(0.5f));
    roundedPath.closeSubPath();
    g.fillPath(roundedPath);

    // fill below part
    juce::Path p2;
    p2.startNewSubPath(width * 0.25f, height);
    p2.lineTo(width - 1, height / 2.0f);

    auto csx = juce::jmin(cornerSize, width * 0.5f);
    auto csy = juce::jmin(cornerSize, height * 0.5f);
    auto cs45x = csx * 0.45f;
    auto cs45y = csy * 0.45f;
    auto x2 = width;
    auto y2 = height;

    p2.lineTo(x2, y2 - csy);
    p2.cubicTo(x2, y2 - cs45y, x2 - cs45x, y2, x2 - csx, y2);
    p2.closeSubPath();

    g.fillPath(p2);
}

juce::Font HighPassButtonLnf::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    return juce::Font(KNOB_FONT, "Regular", KNOB_FONT_SIZE * windowScale);
}

void LowPassButtonLnf::drawButtonBackground(juce::Graphics& g,
    juce::Button& button,
    const juce::Colour& backgroundColour,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    auto cornerSize = 10.0f * windowScale;
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);

    auto baseColour = backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 1.0f)
        .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);

    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        baseColour = baseColour.contrasting(shouldDrawButtonAsDown ? 0.2f : 0.05f);

    g.setColour(baseColour);

    auto flatOnLeft = button.isConnectedOnLeft();
    auto flatOnRight = button.isConnectedOnRight();
    auto flatOnTop = button.isConnectedOnTop();
    auto flatOnBottom = button.isConnectedOnBottom();

    if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom)
    {
        juce::Path path;
        path.addRoundedRectangle(bounds.getX(), bounds.getY(),
            bounds.getWidth(), bounds.getHeight(),
            cornerSize, cornerSize,
            !(flatOnLeft || flatOnTop),
            !(flatOnRight || flatOnTop),
            !(flatOnLeft || flatOnBottom),
            !(flatOnRight || flatOnBottom));

        g.fillPath(path);

        g.setColour(button.findColour(juce::ComboBox::outlineColourId));
        g.strokePath(path, juce::PathStrokeType(1.0f));
    }
    else
    {
        g.fillRoundedRectangle(bounds, cornerSize);
        //g.fillRect(bounds);
        g.setColour(button.findColour(juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
        //g.drawRect(bounds);
    }

    // draw low filter path
    juce::Colour textOnColour = button.findColour(juce::TextButton::textColourOnId);
    juce::Colour textOffColour = button.findColour(juce::TextButton::textColourOffId);
    juce::Colour textColour;

    if (button.getToggleState())
        textColour = textOnColour;
    else
        textColour = textOffColour;

    if (button.isEnabled()) g.setColour(textColour);
    else g.setColour(textColour.darker(0.5f));
    juce::Path p;
    float width = bounds.getWidth();
    float height = bounds.getHeight();
    p.startNewSubPath(1.0f, height / 2.0f);
    p.lineTo(width / 2.0f, height / 2.0f);
    p.lineTo(width * 0.75f, height);
    juce::Path roundedPath = p.createPathWithRoundedCorners(5.0f);
    g.strokePath(roundedPath, juce::PathStrokeType(1.0f));

    // fill above part
    if (button.isEnabled()) g.setColour(textColour.withAlpha(0.5f));
    else g.setColour(textColour.withAlpha(0.5f).darker(0.5f));
    roundedPath.closeSubPath();
    g.fillPath(roundedPath);

    // fill below part
    juce::Path p2;
    p2.startNewSubPath(1.0f, height / 2.0f);
    p2.lineTo(width * 0.75f, height);

    auto csx = juce::jmin(cornerSize, width * 0.5f);
    auto csy = juce::jmin(cornerSize, height * 0.5f);
    auto cs45x = csx * 0.45f;
    auto cs45y = csy * 0.45f;

    auto y2 = height;

    p2.lineTo(csx, y2);
    p2.cubicTo(cs45x, y2, 0, y2 - cs45y, 0, y2 - csy);
    p2.closeSubPath();

    g.fillPath(p2);
}

juce::Font LowPassButtonLnf::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    return juce::Font(KNOB_FONT, "Regular", KNOB_FONT_SIZE * windowScale);
}

void BandPassButtonLnf::drawButtonBackground(juce::Graphics& g,
    juce::Button& button,
    const juce::Colour& backgroundColour,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    auto cornerSize = 10.0f * windowScale;
    auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);

    auto baseColour = backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 1.0f)
        .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);

    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        baseColour = baseColour.contrasting(shouldDrawButtonAsDown ? 0.2f : 0.05f);

    g.setColour(baseColour);

    auto flatOnLeft = button.isConnectedOnLeft();
    auto flatOnRight = button.isConnectedOnRight();
    auto flatOnTop = button.isConnectedOnTop();
    auto flatOnBottom = button.isConnectedOnBottom();

    if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom)
    {
        juce::Path path;
        path.addRoundedRectangle(bounds.getX(), bounds.getY(),
            bounds.getWidth(), bounds.getHeight(),
            cornerSize, cornerSize,
            !(flatOnLeft || flatOnTop),
            !(flatOnRight || flatOnTop),
            !(flatOnLeft || flatOnBottom),
            !(flatOnRight || flatOnBottom));

        g.fillPath(path);

        g.setColour(button.findColour(juce::ComboBox::outlineColourId));
        g.strokePath(path, juce::PathStrokeType(1.0f));
    }
    else
    {
        g.fillRoundedRectangle(bounds, cornerSize);
        g.setColour(button.findColour(juce::ComboBox::outlineColourId));
        g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
    }

    // draw band filter path
    juce::Colour textOnColour = button.findColour(juce::TextButton::textColourOnId);
    juce::Colour textOffColour = button.findColour(juce::TextButton::textColourOffId);
    juce::Colour textColour;

    if (button.getToggleState())
        textColour = textOnColour;
    else
        textColour = textOffColour;

    if (button.isEnabled()) g.setColour(textColour);
    else g.setColour(textColour.darker(0.5f));
    juce::Path p;
    float width = bounds.getWidth();
    float height = bounds.getHeight();
    p.startNewSubPath(width * 0.25f, height);
    p.lineTo(width / 2.0f, height * 0.4f);
    p.lineTo(width * 0.75f, height);
    juce::Path roundedPath = p.createPathWithRoundedCorners(20.0f);
    g.strokePath(roundedPath, juce::PathStrokeType(1.0f));

    if (button.isEnabled()) g.setColour(textColour.withAlpha(0.5f));
    else g.setColour(textColour.withAlpha(0.5f).darker(0.5f));
    g.fillPath(roundedPath);
}

juce::Font BandPassButtonLnf::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    return juce::Font(KNOB_FONT, "Regular", KNOB_FONT_SIZE * windowScale);
}