#pragma once

#include <vector>
#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>

#include "NoteToColor.h"


class WaveTableComponent : public NoteColorListener,
                           public juce::Component
{
public:

    WaveTableComponent(size_t tableSize)
    :   tableSize { tableSize }
    {
        waveform.resize(tableSize, 0.0f); // start at silence or a sine if you want
        setMouseCursor(juce::MouseCursor::CrosshairCursor);
    }

    // callback: editor/processor can hook this to push updates into the synth
    std::function<void(const std::vector<float>&)> onWaveformChanged;

    const std::vector<float>& getWaveform() const noexcept { return waveform; }

    void setWaveform(const std::vector<float>& newWave)
    {
        jassert((int)newWave.size() == tableSize);
        waveform = newWave;
        repaint();

        if (onWaveformChanged)
            onWaveformChanged(waveform);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        auto w = bounds.getWidth();
        auto h = bounds.getHeight();

        g.fillAll(juce::Colours::black);

        // midline
        g.setColour(juce::Colours::dimgrey);
        auto midY = bounds.getCentreY();
        g.drawLine(bounds.getX(), midY, bounds.getRight(), midY);

        if (waveform.empty())
            return;

        juce::Path p;
        bool started = false;

        for (int i = 0; i < tableSize; ++i)
        {
            float xNorm = (float)i / (float)(tableSize - 1); // 0..1
            float x = bounds.getX() + xNorm * w;

            float v = juce::jlimit(-1.0f, 1.0f, waveform[(size_t)i]);
            // map -1..1 to bottom..top
            float y = juce::jmap(v, -1.0f, 1.0f,
                bounds.getBottom(), bounds.getY());

            if (!started)
            {
                p.startNewSubPath(x, y);
                started = true;
            }
            else
            {
                p.lineTo(x, y);
            }
        }

        g.setColour(noteToColorMap[currentNoteColor].withAlpha(0.8f));
        g.strokePath(p,
            juce::PathStrokeType(2.0f,
                juce::PathStrokeType::curved,
                juce::PathStrokeType::rounded));
    }

    // drawing with the mouse
    void mouseDown(const juce::MouseEvent& e) override
    {
        lastIndex = -1;
        applyPoint(e.position);
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        applyPoint(e.position);
    }

    void noteColorChanged(NoteColors newColor) override
    {
        currentNoteColor = newColor;
    }

private:
    NoteColors currentNoteColor = NoteColors::WHITE;
    size_t tableSize;
    std::vector<float> waveform;
    int lastIndex = -1;

    void applyPoint(juce::Point<float> pos)
    {
        auto bounds = getLocalBounds().toFloat();
        auto w = bounds.getWidth();
        auto h = bounds.getHeight();

        if (w <= 0 || h <= 0)
            return;

        float xNorm = juce::jlimit(0.0f, 1.0f,
            (pos.x - bounds.getX()) / w);
        float yNorm = juce::jlimit(0.0f, 1.0f,
            (pos.y - bounds.getY()) / h);

        int index = (int)std::round(xNorm * (tableSize - 1));

        // map y (top=0, bottom=1) to amplitude (1..-1)
        float value = juce::jmap(yNorm, 0.0f, 1.0f, 1.0f, -1.0f);

        // draw a *continuous* line of samples between lastIndex and index
        if (lastIndex < 0 || lastIndex == index)
        {
            waveform[(size_t)index] = value;
        }
        else
        {
            int start = juce::jmin(lastIndex, index);
            int end = juce::jmax(lastIndex, index);

            for (int i = start; i <= end; ++i)
            {
                float t = (float)(i - start) / (float)juce::jmax(1, end - start);
                waveform[(size_t)i] =
                    juce::jmap(t, 0.0f, 1.0f,
                        waveform[(size_t)lastIndex], value);
            }
        }

        lastIndex = index;

        repaint();

        if (onWaveformChanged)
            onWaveformChanged(waveform);
    }
};
