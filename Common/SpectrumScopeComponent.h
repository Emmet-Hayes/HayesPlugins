#pragma once

#include <cmath>
#include <vector>

#include "AudioBufferQueue.h"
#include "CustomLookAndFeel.h"
#include "NoteToColor.h"

constexpr int NUM_PREV_FRAMES_SPECTRUM = 2;

template <typename SampleType>
class SpectrumScopeComponent : public NoteColorListener,
    public juce::Component,
    private juce::Timer
{
public:
    SpectrumScopeComponent(AudioBufferQueue<SampleType>& queueToUse,
        CustomLookAndFeel& laf)
        : lookAndFeel{ laf }
        , audioBufferQueue{ queueToUse }
    {
        resizeInternalBuffersFromQueue();
        setFramesPerSecond(60);
    }

    void resizeInternalBuffersFromQueue()
    {
        const auto blockSize = audioBufferQueue.getBufferSize();
        if (blockSize == 0)
            return;

        // For FFT, we assume blockSize is a power of two:
        jassert((blockSize & (blockSize - 1)) == 0); // power-of-two check

        const int fftOrder = (int)std::log2((double)blockSize);
        const auto fftSize = (size_t)blockSize;

        buffer.assign(blockSize, SampleType(0));

        spectrumData.assign(2 * fftSize, SampleType(0));
        prevFrames.clear();
        prevFrames.resize(NUM_PREV_FRAMES_SPECTRUM,
            std::vector<SampleType>(spectrumData.size(), SampleType(0)));

        fft = std::make_unique<juce::dsp::FFT>(fftOrder);
        windowFun = std::make_unique<juce::dsp::WindowingFunction<SampleType>>(
            fftSize, juce::dsp::WindowingFunction<SampleType>::hann);

        currentFrame = 0;
    }

    void noteColorChanged(NoteColors newColor) override
    {
        currentNoteColor = newColor;
    }

    void setFramesPerSecond(int framesPerSecond)
    {
        jassert(framesPerSecond > 0 && framesPerSecond < 1000);
        startTimerHz(framesPerSecond);
    }

    void paint(juce::Graphics& g) override
    {
        auto area = getLocalBounds();
        auto h = (SampleType)area.getHeight();
        auto w = (SampleType)area.getWidth();

        auto spectrumRect = juce::Rectangle<SampleType>{
            SampleType(20), SampleType(0), w, h - 20
        };

        // draw grid
        g.setColour(juce::Colours::darkturquoise);
        g.setFont(lookAndFeel.getCommonMenuFont(scaleH));

        const int numVerticalLines = 10;
        const int numHorizontalLines = 8;

        for (int i = 1; i < numVerticalLines; ++i)
        {
            float x = juce::jmap<float>(
                static_cast<float>(i),
                0.0f, static_cast<float>(numVerticalLines),
                spectrumRect.getX(), spectrumRect.getRight());

            g.drawLine(x, spectrumRect.getY(), x, spectrumRect.getBottom());
            g.drawText(xLabels[i - 1],
                static_cast<int>(x - (5 * scaleW)),
                static_cast<int>(spectrumRect.getBottom() - (5 * scaleH)),
                static_cast<int>(30 * scaleW),
                static_cast<int>(20 * scaleH),
                juce::Justification::left);
        }

        for (int i = 1; i < numHorizontalLines; ++i)
        {
            float y = juce::jmap<float>(
                static_cast<float>(i),
                0.0f, static_cast<float>(numHorizontalLines),
                spectrumRect.getY(), spectrumRect.getBottom());

            g.drawLine(spectrumRect.getX(), y, spectrumRect.getRight(), y);
            g.drawText(ampLabels[i - 1],
                0,
                static_cast<int>(y - (10 * scaleH)),
                static_cast<int>(30 * scaleW),
                static_cast<int>(20 * scaleH),
                juce::Justification::left);
        }

        if (spectrumData.empty())
            return;

        // 8 probes for changes in amplitude – if any are "hot", draw all frames
        bool shouldDraw = false;
        const auto step = spectrumData.size() / 8;
        for (int j = 0; j < 8; ++j)
        {
            auto v = spectrumData[j * step];
            if (v > 0.1f || v < -10.1f)
            {
                shouldDraw = true;
                break;
            }
        }

        if (!shouldDraw)
            return;

        for (size_t i = 0; i < prevFrames.size(); ++i)
        {
            float opacity = 0.3f + 0.7f * (float(i) / float(prevFrames.size()));

            juce::ColourGradient grad(
                noteToColorMap[currentNoteColor].withAlpha(opacity), 0.0f, 0.0f,
                juce::Colours::pink.withAlpha(opacity), 0.0f,
                (float)getLocalBounds().getHeight(), false);

            g.setGradientFill(grad);

            const auto& frame = prevFrames[i];
            if (!frame.empty())
                plot(frame.data(),
                    frame.size() / 4, // same as before
                    g, spectrumRect);
        }
    }

    void setScale(float newScaleW, float newScaleH)
    {
        scaleW = newScaleW;
        scaleH = newScaleH;
    }

    void resized() override {}

private:
    NoteColors              currentNoteColor = NoteColors::WHITE;
    CustomLookAndFeel& lookAndFeel;
    AudioBufferQueue<SampleType>& audioBufferQueue;

    std::vector<SampleType>                   buffer;
    std::vector<std::vector<SampleType>>      prevFrames;
    int                                       currentFrame = 0;

    std::unique_ptr<juce::dsp::FFT>                          fft;
    std::unique_ptr<juce::dsp::WindowingFunction<SampleType>> windowFun;
    std::vector<SampleType>                                  spectrumData;

    const char* xLabels[9]{ "46", "92", "184", "368", "736", "1.5k", "3k", "6k", "12k" };
    const char* ampLabels[7]{ " 15", " 10", "  5", "  0", " -5", "-10", "-15" };

    float scaleW = 1.0f;
    float scaleH = 1.0f;

    void timerCallback() override
    {
        if (!fft || !windowFun)
            return;

        bool gotData = false;
        while (audioBufferQueue.pop(buffer.data()))
            gotData = true;

        if (!gotData || buffer.empty())
        {
            repaint();
            return;
        }

        const auto fftSize = (size_t)fft->getSize();
        jassert(fftSize <= buffer.size());
        jassert(spectrumData.size() == 2 * fftSize);

        // copy time-domain samples into the first half of spectrumData
        juce::FloatVectorOperations::copy(
            spectrumData.data(),
            buffer.data(),
            (int)fftSize);

        // clear the imag part (second half)
        juce::FloatVectorOperations::clear(
            spectrumData.data() + (int)fftSize,
            (int)fftSize);

        windowFun->multiplyWithWindowingTable(spectrumData.data(), fftSize);
        fft->performFrequencyOnlyForwardTransform(spectrumData.data());

        static constexpr auto mindB = SampleType(-160);
        static constexpr auto maxdB = SampleType(0);

        for (auto& s : spectrumData)
        {
            s = juce::jmap(
                juce::jlimit(mindB, maxdB,
                    juce::Decibels::gainToDecibels(s)
                    - juce::Decibels::gainToDecibels(
                        SampleType(fftSize))),
                mindB, maxdB,
                SampleType(0), SampleType(1));
        }

        if (!prevFrames.empty())
        {
            prevFrames[(size_t)currentFrame] = spectrumData;
            currentFrame = (currentFrame + 1) % NUM_PREV_FRAMES_SPECTRUM;
        }

        repaint();
    }

    static void plot(const SampleType* data,
        size_t numSamples,
        juce::Graphics& g,
        juce::Rectangle<SampleType> rect,
        SampleType scale = SampleType(0.7),
        SampleType offset = SampleType(0))
    {
        auto w = rect.getWidth();
        auto h = rect.getHeight();
        auto center = rect.getBottom() - offset;
        auto gain = h * scale;
        auto left = rect.getX();

        SampleType minFreq = 20;
        SampleType maxFreq = 20000;

        SampleType logScaleFactor =
            w / (std::log10(maxFreq) - std::log10(minFreq));

        juce::Path plotPath;

        SampleType firstFreq = juce::jmap(SampleType(0),
            SampleType(0), SampleType(numSamples),
            minFreq, maxFreq);
        SampleType firstXPos =
            (std::log10(firstFreq) - std::log10(minFreq)) * logScaleFactor;
        plotPath.startNewSubPath(left + firstXPos, center - gain * data[0]);

        for (size_t i = 1; i < numSamples; ++i)
        {
            SampleType freq = juce::jmap(SampleType(i),
                SampleType(0), SampleType(numSamples),
                minFreq, maxFreq);
            SampleType xPos =
                (std::log10(freq) - std::log10(minFreq)) * logScaleFactor;

            plotPath.lineTo(left + xPos, center - gain * data[i]);
        }

        plotPath.lineTo(left + w, center);
        plotPath.lineTo(left, center);
        plotPath.closeSubPath();

        g.setOpacity(0.55f);
        g.fillPath(plotPath);

        // Add a thin outline for extra clarity
        g.setOpacity(0.85f);
        g.strokePath(plotPath, juce::PathStrokeType(1.0f));
    }
};

template <typename SampleType>
class SpectrumScopeDataCollector
{
public:
    SpectrumScopeDataCollector(AudioBufferQueue<SampleType>& queueToUse)
    :   audioBufferQueue(queueToUse)
    {
        resizeInternalBufferFromQueue();
    }

    void resizeInternalBufferFromQueue()
    {
        const auto size = audioBufferQueue.getBufferSize();
        if (size == 0)
            return;

        buffer.assign(size, SampleType(0));
        numCollected = 0;
        prevSample = SampleType(100);
        state = State::waitingForTrigger;
    }

    void addListener(NoteColorListener* listener)
    {
        listeners.push_back(listener);
    }

    void removeAllListeners()
    {
        listeners.clear();
    }

    void process(const SampleType* data, size_t numSamples)
    {
        size_t index = 0;

        if (state == State::waitingForTrigger)
        {
            while (index++ < numSamples)
            {
                auto currentSample = *data++;
                if (std::fabs(currentSample) >= triggerLevel
                    && prevSample < triggerLevel)
                {
                    numCollected = 0;
                    state = State::collecting;
                    break;
                }
                prevSample = currentSample;
            }
        }

        if (state == State::collecting)
        {
            while (index++ < numSamples && numCollected < buffer.size())
            {
                buffer[numCollected++] = *data++;
                if (numCollected == buffer.size())
                {
                    (void)audioBufferQueue.push(buffer.data(), buffer.size());
                    state = State::waitingForTrigger;
                    prevSample = SampleType(100);
                    break;
                }
            }
        }
    }

    void process(const SampleType* data, size_t numSamples, float minFrequency)
    {
        process(data, numSamples);

        for (auto* listener : listeners)
            if (listener != nullptr)
                listener->noteColorChanged(frequencyToColor(minFrequency));
    }

private:
    std::vector<NoteColorListener*> listeners;
    AudioBufferQueue<SampleType>& audioBufferQueue;

    std::vector<SampleType> buffer;
    size_t                  numCollected = 0;
    SampleType              prevSample = SampleType(100);
    static constexpr auto   triggerLevel = SampleType(0.01);

    enum class State { waitingForTrigger, collecting } state { State::waitingForTrigger };
};

