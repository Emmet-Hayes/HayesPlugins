#include "BaseAudioProcessorEditor.h"


BaseAudioProcessorEditor::BaseAudioProcessorEditor(BaseAudioProcessor& p)
:   juce::AudioProcessorEditor { p }
,   processor { p }
{
}
