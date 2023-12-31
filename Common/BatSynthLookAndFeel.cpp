#include "BatSynthLookAndFeel.h"


void BatSynthLookAndFeel::setGainColorIntensity(float value) 
{
    rotaryOutlineBrightness = value;
}

void BatSynthLookAndFeel::setFrequencyColor(float freq) 
{
    currentNoteColor = frequencyToColor(freq);
}

void BatSynthLookAndFeel::drawRotarySlider(Graphics& g, int x, int y, int width, int height,
                                  float sliderPos, const float rotaryStartAngle, const float rotaryEndAngle, 
                                  Slider& slider) 
{
    double radius = jmin(width / 2, height / 2) - 4.0f;
    double centreX = x + width * 0.5f;
    double centreY = y + height * 0.5f;
    double rx = centreX - radius;
    double ry = centreY - radius;
    double rw = radius * 2.0f;
    double angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    
    g.setColour(Colours::whitesmoke);
    g.setOpacity(0.33f);
    g.fillEllipse(rx, ry, rw, rw);
  
    g.setColour(noteToColorMap[currentNoteColor].withMultipliedBrightness(rotaryOutlineBrightness * 10));
    g.setOpacity(1.0f);
    g.drawEllipse(rx, ry, rw, rw, 2.0f);
  
    Path p;
    double pointerLength = radius * 0.5f;
    double pointerThickness = 5.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform(AffineTransform::rotation(angle).translated(centreX, centreY)); //animate
    g.setColour(Colours::seashell);
    g.fillPath(p);
}