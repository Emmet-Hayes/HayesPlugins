#pragma once
#include <JuceHeader.h>

class ModSlider : public juce::Slider
{
public:
    ModSlider(juce::Label* l) : label(l), isModifiable(false)
    {
    }

    ~ModSlider() { label = nullptr; }

    void mouseDown(const juce::MouseEvent& e) override
    {
        const juce::ModifierKeys modifiers = juce::ModifierKeys::getCurrentModifiersRealtime();
        if (isModifiable && modifiers.isPopupMenu())
        {
            if (isEnabled() && label->isEnabled())
            {
                setEnabled(false);
                label->setEnabled(false);
            }
            else
            {
                setEnabled(true);
                label->setEnabled(true);
            }
        }
        else
            Slider::mouseDown(e);
    }

    void setHasModifiers(bool mod)
    {
        isModifiable = mod;
    }

    bool hasModifier() const
    {
        return isModifiable;
    }


private:
    juce::Label* label;
    bool isModifiable;
};
