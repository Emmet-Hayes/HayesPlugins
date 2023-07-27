#pragma once

#include <JuceHeader.h>

#include "../../../GUI/InterfaceDefines.h"


class GraphTemplate : public juce::Component
{
public:
    GraphTemplate() {}

    void paint (juce::Graphics&) override;
    void resized() override;
    void setScale (float pscale);
    float getScale();
    bool getZoomState();
    void setZoomState (bool zoomState);
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseEnter (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;

protected:
    bool isMouseOn = false;
    bool mZoomState = false;

private:
    float scale = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphTemplate)
};
