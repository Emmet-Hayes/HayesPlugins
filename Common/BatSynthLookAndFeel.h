#pragma once

#include "CustomLookAndFeel.h"
#include "NoteToColor.h"


class BatSynthLookAndFeel : public CustomLookAndFeel
{
public:
	void drawRotarySlider(Graphics&, int, int, int, int, float, const float, 
						  const float, Slider&) override;

	void setGainColorIntensity(float ampl);
	void setFrequencyColor(float freq);

private:
	float rotaryOutlineBrightness { 1.0f };
	NoteColors currentNoteColor { NoteColors::WHITE };
};