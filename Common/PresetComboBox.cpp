#include "PresetBar.h"
#include "PresetComboBox.h"

PresetComboBox::PresetComboBox(BaseAudioProcessor& p, PresetBar* bar)
    : processor{ p }
    , presetBar{ bar }
{
    saveLoadMenu.addItem("Save Preset", [this]() { savePreset(); });
    saveLoadMenu.addItem("Load Preset", [this]() { loadPreset(); });
    saveLoadMenu.addItem("Delete Preset", [this]() { deletePreset(); });
}

void PresetComboBox::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isRightButtonDown())
        saveLoadMenu.showMenuAsync(juce::PopupMenu::Options());
    else
        juce::ComboBox::mouseDown(event);
}

void PresetComboBox::savePreset()
{
    juce::FileChooser chooser("Select a file to save...", {}, "*.preset");

    if (chooser.browseForFileToSave(true))
    {
        juce::File file(chooser.getResult());

        // Save the current processor state to the chosen file
        auto state = processor.apvts.copyState();
        if (state.isValid())
        {
            // Create the file if it doesn't exist
            file.create();

            // Create a FileOutputStream for the preset file
            std::unique_ptr<juce::FileOutputStream> outStream(file.createOutputStream());

            if (outStream != nullptr)
            {
                // Write the state ValueTree into the file
                state.writeToStream(*outStream);
            }
        }
    }
}

void PresetComboBox::loadPreset()
{
    juce::FileChooser chooser("Select a preset to load...", {}, "*.preset");

    if (chooser.browseForFileToOpen())
    {
        juce::File file(chooser.getResult());

        // Load the state from the chosen file into the processor
        // Create a FileInputStream for the preset file
        std::unique_ptr<juce::FileInputStream> inStream(file.createInputStream());

        if (inStream != nullptr)
        {
            // Read the ValueTree state from the file
            juce::ValueTree state = juce::ValueTree::readFromStream(*inStream);

            processor.apvts.replaceState(state);
            auto newPresetName = file.getFileNameWithoutExtension().toStdString();
            presetBar->addPreset(newPresetName);
            presetBar->presetBox.addItem(newPresetName, presetBar->presetBox.getNumItems() + 1);
            presetBar->presetBox.setSelectedId(presetBar->presetBox.getNumItems(), juce::dontSendNotification);
            presetBar->presetChanged();

        }
    }
}

void PresetComboBox::deletePreset()
{
    auto presetName = getText().toStdString();
    if (presetName.empty())
        return;
    
    auto result = juce::AlertWindow::showOkCancelBox(juce::AlertWindow::QuestionIcon,
        "Delete Preset", "Do you also want to delete the preset file from disk?", "Yes", "No");

    presetBar->deletePreset(presetName, result);
}
