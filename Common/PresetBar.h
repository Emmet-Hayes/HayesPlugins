#pragma once
#include <JuceHeader.h>
#include "BaseAudioProcessor.h"
#include "PresetComboBox.h"

class PresetBar : public juce::Component
                , public juce::Button::Listener
{
public:
    PresetBar(BaseAudioProcessor& p);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void buttonClicked(juce::Button* b) override;

    void addPresetButtonClicked();
    void presetChanged();

    juce::TextButton nextButton{ ">" };
    juce::TextButton prevButton{ "<" };
    juce::TextButton addPresetButton{ "+" };
    PresetComboBox presetBox;

    void addPreset(const std::string& name);
    void deletePreset(const std::string& name, bool alsoDeleteFile = false);

private:
    void loadPreset(const std::string& name);
    bool loadDefaultPresets();
    bool savePreset(const std::string& name);

    int getNextPresetIndex();
    int getPrevPresetIndex();
    std::string getNextPresetName();
    std::string getPrevPresetName();


    std::vector<std::pair<std::string, juce::ValueTree>> presets;
    std::string currentPresetName;
    int currentPresetIndex;
    BaseAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetBar)
};
