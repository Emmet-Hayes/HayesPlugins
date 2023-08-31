#pragma once

#include "BaseAudioProcessor.h"


class PresetBar;

class PresetComboBox : public juce::ComboBox
{
public:
    PresetComboBox(BaseAudioProcessor& p, PresetBar* bar);
    void mouseDown(const juce::MouseEvent& event) override;
    void savePreset();
    void loadPreset();
    void deletePreset();

    BaseAudioProcessor& processor;
    PresetBar* presetBar;
    juce::PopupMenu saveLoadMenu;
};
