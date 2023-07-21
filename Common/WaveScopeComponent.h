#pragma once
#include <cmath>

constexpr int NUM_PREV_FRAMES = 4;

template <typename SampleType>
class AudioBufferQueue
{
public:
	static constexpr size_t order = 10;
	static constexpr size_t bufferSize = 1U << order;
	static constexpr size_t numBuffers = 5;

	void push(const SampleType* dataToPush, size_t numSamples)
	{
		jassert(numSamples <= bufferSize);
		int start1, size1, start2, size2;
		abstractFifo.prepareToWrite(1, start1, size1, start2, size2);
		jassert(size1 <= 1);
		jassert(size2 == 0);
		if (size1 > 0)
			FloatVectorOperations::copy(buffers[(size_t)start1].data(), dataToPush, (int)jmin(bufferSize, numSamples));
		abstractFifo.finishedWrite(size1);
	}

	void pop(SampleType* outputBuffer)
	{
		int start1, size1, start2, size2;
		abstractFifo.prepareToRead(1, start1, size1, start2, size2);
		jassert(size1 <= 1);
		jassert(size2 == 0);
		if (size1 > 0) FloatVectorOperations::copy(outputBuffer, buffers[(size_t)start1].data(), (int)bufferSize);
		abstractFifo.finishedRead(size1);
	}

private:
	AbstractFifo abstractFifo{ numBuffers };
	std::array<std::array<SampleType, bufferSize>, numBuffers> buffers;
};

template <typename SampleType>
class WaveScopeComponent : public juce::Component
	                 , private Timer
{
public:
	WaveScopeComponent(AudioBufferQueue<SampleType>& queueToUse)
		: audioBufferQueue(queueToUse)
	{
		buffer.fill(SampleType(0));
		setFramesPerSecond(60);
	}

	void setFramesPerSecond(int framesPerSecond)
	{
		jassert(framesPerSecond > 0 && framesPerSecond < 1000);
		startTimerHz(framesPerSecond);
	}

	void paint(Graphics& g) override
	{
		auto area = getLocalBounds();
		auto h = (SampleType)area.getHeight();
		auto w = (SampleType)area.getWidth();

		g.setColour(juce::Colours::darkturquoise.withBrightness(0.2f));
		g.setOpacity(0.7f);
		g.fillRoundedRectangle({ 0, 0, w, h }, 20);

		g.setColour(juce::Colours::white);
		g.setOpacity(1.0f);

		
		for (size_t i = 0; i < prevFrames.size(); ++i)
		{
			float opacity = 0.1f + 0.9f * (i / (float)prevFrames.size());
			g.setColour(juce::Colours::hotpink.withAlpha(opacity));
			auto scopeRect = juce::Rectangle<SampleType>{ SampleType(0), SampleType(0), w, h };
			plot(prevFrames[i].data(), prevFrames[i].size(), g, scopeRect, SampleType(1), h / 2);
		}
	}

	void resized() override {}

private:
	AudioBufferQueue<SampleType>& audioBufferQueue;
	std::array<SampleType, AudioBufferQueue<SampleType>::bufferSize> buffer;
	std::array<std::array<SampleType, AudioBufferQueue<SampleType>::bufferSize>, NUM_PREV_FRAMES> prevFrames;
	juce::dsp::FFT fft{ AudioBufferQueue<SampleType>::order };
	juce::dsp::WindowingFunction<SampleType> windowFun{ (size_t)fft.getSize(), juce::dsp::WindowingFunction<SampleType>::hann };
	std::array<SampleType, AudioBufferQueue<SampleType>::bufferSize> scopeData;
	int currentFrame = 0;

	void timerCallback() override
	{
		audioBufferQueue.pop(buffer.data());
		FloatVectorOperations::copy(scopeData.data(), buffer.data(), (int)buffer.size());
		prevFrames[currentFrame++] = scopeData;
		currentFrame %= NUM_PREV_FRAMES;
		repaint();
	}

	static void plot(const SampleType* data, size_t numSamples, Graphics& g, juce::Rectangle<SampleType> rect,
		SampleType scale = SampleType(0.7), SampleType offset = SampleType(0))
	{
		auto w = rect.getWidth();
		auto h = rect.getHeight();
		auto right = rect.getRight();
		auto center = rect.getBottom() - offset;
		auto gain = h * scale;
		for (size_t i = 1; i < numSamples; ++i)
		{
			auto yPosStart = center - gain * data[i - 1];
			auto yPosEnd = center - gain * data[i];

			// Check if both y positions are within the rectangle's bounds
			if (yPosStart >= rect.getY() && yPosStart <= rect.getBottom() &&
				yPosEnd >= rect.getY() && yPosEnd <= rect.getBottom())
			{
				g.drawLine({ jmap(SampleType(i - 1), SampleType(0), SampleType(numSamples - 1), SampleType(right - w),
					SampleType(right)), yPosStart, jmap(SampleType(i), SampleType(0),
					SampleType(numSamples - 1), SampleType(right - w), SampleType(right)), yPosEnd });
			}
		}
	}
};

template <typename SampleType>
class WaveScopeDataCollector
{
public:
	WaveScopeDataCollector (AudioBufferQueue<SampleType>& queueToUse)
    :   audioBufferQueue(queueToUse)
	{}

	void process(const SampleType* data, size_t numSamples)
	{
		size_t index = 0;	 
		if (state == State::waitingForTrigger)
		{
			while (index++ < numSamples)
			{
				auto currentSample = *data++;
				if (fabs(currentSample) >= triggerLevel && prevSample < triggerLevel)
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
			while (index++ < numSamples)
			{
				buffer[numCollected++] = *data++;
				if (numCollected == buffer.size())
				{
					audioBufferQueue.push(buffer.data(), buffer.size());
					state = State::waitingForTrigger;
					prevSample = SampleType(100);
					break;
				}
			}
		}
	}

private:
	AudioBufferQueue<SampleType>& audioBufferQueue;
	std::array<SampleType, AudioBufferQueue<SampleType>::bufferSize> buffer;
	size_t numCollected;
	SampleType prevSample = SampleType(100);
	static constexpr auto triggerLevel = SampleType(0.01);
	enum class State { waitingForTrigger, collecting } state{ State::waitingForTrigger };
};
