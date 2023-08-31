#include "PresetBar.h"
#include "PresetComboBox.h"

PresetBar::PresetBar(BaseAudioProcessor& p)
:   presetBox { p , this }
,   currentPresetName { "Default" }
,   currentPresetIndex { 0 }
,   processor { p }
{
    nextButton.addListener(this);
    prevButton.addListener(this);
    addPresetButton.addListener(this);
    addAndMakeVisible(nextButton);
    addAndMakeVisible(prevButton);
    addAndMakeVisible(addPresetButton);
    addAndMakeVisible(presetBox);

    presetBox.onChange = [this] { presetChanged(); };

    if (!loadDefaultPresets())
    {
        addPreset("Default");
    }
}

void PresetBar::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkslateblue);
}

void PresetBar::resized()
{
    auto area = getLocalBounds();
    addPresetButton.setBounds(area.removeFromRight(proportionOfWidth(0.1f)));
    nextButton.setBounds(area.removeFromRight(proportionOfWidth(0.1f)));
    prevButton.setBounds(area.removeFromRight(proportionOfWidth(0.1f)));
    presetBox.setBounds(area);
}

void PresetBar::buttonClicked(juce::Button* b)
{
    if (b == &nextButton)
    {
        if (presets.size() > 0)
        {
            auto nextPresetName = getNextPresetName();
            auto nextPresetIndex = getNextPresetIndex();
            if (!nextPresetName.empty())
            {
                loadPreset(nextPresetName);
                presetBox.setSelectedId(presetBox.getItemId(nextPresetIndex), juce::dontSendNotification);
            }
        }
    }
    else if (b == &prevButton)
    {
        if (presets.size() > 0)
        {
            auto prevPresetName = getPrevPresetName();
            auto prevPresetIndex = getPrevPresetIndex();
            if (!prevPresetName.empty())
            {
                loadPreset(prevPresetName);
                presetBox.setSelectedId(presetBox.getItemId(prevPresetIndex), juce::dontSendNotification);
            }
        }
    }
    else if (b == &addPresetButton)
    {
        addPresetButtonClicked();
    }
}

void PresetBar::addPresetButtonClicked()
{
    auto alertWindow = std::make_unique<juce::AlertWindow>("New Preset", "Enter a name for the new preset.", juce::AlertWindow::NoIcon);
    alertWindow->addTextEditor("Name", "", "Preset name:");
    alertWindow->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
    alertWindow->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    auto* proc = &processor;

    alertWindow->enterModalState(true, juce::ModalCallbackFunction::create(
        [proc, this, w = alertWindow.release()](int result) mutable
    {
        if (result == 1)
        {
            auto newPresetName = w->getTextEditorContents("Name").toStdString();
            addPreset(newPresetName);
            presetBox.addItem(newPresetName, presetBox.getNumItems() + 1);
            presetBox.setSelectedId(presetBox.getNumItems(), juce::dontSendNotification);
        }
        delete w;
    }
    ));
}

void PresetBar::presetChanged()
{
    currentPresetIndex = presetBox.getSelectedItemIndex();
    currentPresetName = presets[currentPresetIndex].first;
    loadPreset(currentPresetName);
}
void PresetBar::addPreset(const std::string& name)
{
    auto state = processor.apvts.copyState();
    presets.emplace_back(name, state);

    auto result = juce::AlertWindow::showOkCancelBox(juce::AlertWindow::QuestionIcon,
        "Add Preset", "Do you also want to add the preset file to disk?", "Yes", "No");
    if (result)
    {
        if (!savePreset(name))
            juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon,
                "Add Preset", "Failed to save preset file to disk.");
    }
}

void PresetBar::deletePreset(const std::string& name, bool alsoDeleteFile)
{
    auto it = std::find_if(presets.begin(), presets.end(), [&](const auto& preset) { return preset.first == name; });
    if (it != presets.end())
    {
        if (alsoDeleteFile)
        {
            juce::File presetDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                .getChildFile(JucePlugin_Name);

            juce::File presetFile = presetDir.getChildFile(name + ".preset");

            if (presetFile.exists())
                presetFile.deleteFile();
        }

        presets.erase(it);

        presetBox.clear(juce::dontSendNotification);

        for (const auto& preset : presets)
        {
            presetBox.addItem(preset.first, presetBox.getNumItems() + 1);
        }

        if (!presets.empty())
        {
            loadPreset(presets[0].first);
            presetBox.setSelectedId(1, juce::dontSendNotification);
        }
    }
}



void PresetBar::loadPreset(const std::string& name)
{
    auto it = std::find_if(presets.begin(), presets.end(), [&](const auto& preset) { return preset.first == name; });
    if (it != presets.end())
    {
        processor.apvts.replaceState(it->second);

        int index = static_cast<int>(std::distance(presets.begin(), it));
        presetBox.setSelectedItemIndex(index);
        currentPresetIndex = index;
        currentPresetName = presets[currentPresetIndex].first;
    }
}

bool PresetBar::loadDefaultPresets()
{
    juce::File presetDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile(JucePlugin_Name);

    if (presetDir.exists())
    {
        juce::Array<juce::File> presetFiles;
        presetDir.findChildFiles(presetFiles, juce::File::findFiles, false, "*.preset");

        for (const auto& file : presetFiles)
        {
            std::unique_ptr<juce::FileInputStream> inStream(file.createInputStream());

            if (inStream != nullptr)
            {
                juce::ValueTree state = juce::ValueTree::readFromStream(*inStream);

                auto presetName = file.getFileNameWithoutExtension().toStdString();
                presets.emplace_back(presetName, state);
                presetBox.addItem(presetName, presetBox.getNumItems() + 1);
            }
        }
        presetBox.setSelectedItemIndex(0);
        return true;
    }
    
    return false;

}

bool PresetBar::savePreset(const std::string& name)
{
    auto it = std::find_if(presets.begin(), presets.end(), [&](const auto& preset) { return preset.first == name; });
    if (it == presets.end())
        return false;

    juce::File presetDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile(JucePlugin_Name);

    if (!presetDir.exists())
        presetDir.createDirectory();

    juce::File file(presetDir.getChildFile(name + ".preset"));

    std::unique_ptr<juce::FileOutputStream> outStream(file.createOutputStream());

    if (outStream == nullptr)
        return false;

    it->second.writeToStream(*outStream);

    return true;
}


int PresetBar::getNextPresetIndex()
{
    return currentPresetIndex >= presets.size() - 1 ? 0 : currentPresetIndex + 1;
}

int PresetBar::getPrevPresetIndex()
{
    return (currentPresetIndex + static_cast<int>(presets.size()) - 1) % presets.size();
}

std::string PresetBar::getNextPresetName()
{
    return presets[(currentPresetIndex + (size_t)1) % presets.size()].first;
}

std::string PresetBar::getPrevPresetName()
{
    return presets[(currentPresetIndex + presets.size() - 1) % presets.size()].first;
}

