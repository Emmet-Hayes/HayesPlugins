#pragma once

#include <cmath>

#include "AudioBufferQueue.h"
#include "NoteToColor.h"


constexpr int NUM_PREV_FRAMES = 2;

template <typename SampleType>
class WaveScopeComponent : public NoteColorListener
	                     , public juce::Component
	                     , private Timer
{
public:
	WaveScopeComponent(AudioBufferQueue<SampleType>& queueToUse)
	:   audioBufferQueue(queueToUse)
	{
		resizeInternalBuffersFromQueue();
		setFramesPerSecond(60);
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

	void paint(Graphics& g) override
	{
		auto area = getLocalBounds().toFloat();
		auto h = area.getHeight();
		auto w = area.getWidth();

		g.fillAll(juce::Colours::black);

		const auto numFrames = (int)prevFrames.size();
		if (numFrames == 0 || w <= 0.0f || h <= 0.0f)
			return;

		for (int i = 0; i < numFrames; ++i)
		{
			const float t = (float)i / (float)numFrames;
			const float opacity = 0.1f + 0.9f * t;

			g.setColour(noteToColorMap[currentNoteColor].withAlpha(opacity));

			juce::Rectangle<SampleType> scopeRect { SampleType(0), SampleType(0),
				                                    (SampleType)w, (SampleType)h };

			plot(prevFrames[i].data(), prevFrames[i].size(), g,
				 scopeRect, SampleType(1), (SampleType)(h / 2.0f));
		}
	}

	void resizeInternalBuffersFromQueue()
	{
		const int size = audioBufferQueue.getBufferSize();
		if (size == 0)
			return;

		buffer.assign(size, SampleType(0));
		scopeData.assign(size, SampleType(0));

		prevFrames.clear();
		prevFrames.resize(NUM_PREV_FRAMES);
		for (auto& frame : prevFrames)
			frame.assign(size, SampleType(0));
	}

	void resized() override {}

private:
	NoteColors currentNoteColor = NoteColors::WHITE;
	AudioBufferQueue<SampleType>& audioBufferQueue;
	std::vector<SampleType> buffer;
	std::vector<std::vector<SampleType>> prevFrames;
	std::vector<SampleType> scopeData;
	int currentFrame = 0;

	void timerCallback() override
	{
		while (audioBufferQueue.pop(buffer.data())) {}

		FloatVectorOperations::copy(scopeData.data(), buffer.data(), (int)buffer.size());
		prevFrames[(size_t) currentFrame] = scopeData;
		currentFrame = (currentFrame + 1) % NUM_PREV_FRAMES;

		repaint();
	}



	static void plot(const SampleType* data,
		size_t numSamples,
		Graphics& g,
		juce::Rectangle<SampleType> rect,
		SampleType scale = SampleType(0.7),
		SampleType offset = SampleType(0))
	{
		if (numSamples < 2)
			return;

		const auto w = (int)rect.getWidth();
		const auto h = rect.getHeight();
		if (w <= 0 || h <= SampleType(0))
			return;

		const auto left = rect.getX();
		const auto right = rect.getRight();

		const auto center = rect.getBottom() - offset;
		const auto gain = h * scale;

		const double samplesPerPixel = static_cast<double> (numSamples) / static_cast<double> (w);

		juce::Path path;
		bool started = false;

		for (int x = 0; x < w; ++x)
		{
			// Map pixel x to sample index
			size_t sampleIndex = static_cast<size_t>(std::floor(x * samplesPerPixel));
			if (sampleIndex >= numSamples)
				sampleIndex = numSamples - 1;

			const auto sample = data[sampleIndex];
			const auto y = center - gain * sample;
			const auto xPos = left + static_cast<SampleType>(x);

			if (!started)
			{
				path.startNewSubPath(xPos, y);
				started = true;
			}
			else
			{
				path.lineTo(xPos, y);
			}
		}

		if (!path.isEmpty())
			g.strokePath(path, juce::PathStrokeType(2.0f,
				juce::PathStrokeType::curved,
				juce::PathStrokeType::rounded));
	}
};

template <typename SampleType>
class WaveScopeDataCollector
{
public:
	WaveScopeDataCollector (AudioBufferQueue<SampleType>& queueToUse)
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
		for (int i = 0; i < listeners.size(); ++i)
			listeners.pop_back();
	}

	void process(const SampleType* data, size_t numSamples)
	{
		size_t index = 0;

		if (state == State::waitingForTrigger)
		{
			while (index++ < numSamples)
			{
				auto currentSample = *data++;
				if (std::fabs(currentSample) >= triggerLevel && prevSample < triggerLevel)
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
					// ok to ignore the bool return — drop if queue is full
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
	size_t numCollected = 0;
	SampleType prevSample = SampleType(100);
	static constexpr auto triggerLevel = SampleType(0.01);
	enum class State { waitingForTrigger, collecting } state { State::waitingForTrigger };
};
