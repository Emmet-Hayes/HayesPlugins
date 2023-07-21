#pragma once
#include <cmath>

constexpr int NUM_PREV_FRAMES = 2;

template <typename SampleType>
class AudioBufferQueue 
{
public:
	static constexpr size_t order = 8;
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
class SpectrumScopeComponent : public juce::Component
		             , private Timer
{
public:
	SpectrumScopeComponent(AudioBufferQueue<SampleType>& queueToUse)
	:   audioBufferQueue(queueToUse) 
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

		auto spectrumRect = juce::Rectangle<SampleType>{ SampleType(0), SampleType(0), w, h };

		// draw grid
		g.setColour(juce::Colours::darkturquoise);
		int numVerticalLines = 10;
		int numHorizontalLines = 8;

		for (int i = 1; i < numVerticalLines; ++i)
		{
			float x = jmap<float>(i, 0, numVerticalLines, spectrumRect.getX(), spectrumRect.getRight());
			g.drawLine(x, spectrumRect.getY() + 10, x, spectrumRect.getBottom() - 15);
			g.drawText(xLabels[i - 1], x - 5, spectrumRect.getBottom() - 25, 25, 25, juce::Justification::left);
		}

		for (int i = 1; i < numHorizontalLines; ++i)
		{
			float y = jmap<float>(i, 0, numHorizontalLines, spectrumRect.getY(), spectrumRect.getBottom());
			g.drawLine(spectrumRect.getX() + 20, y, spectrumRect.getRight() - 20, y);
			g.drawText(ampLabels[i - 1], spectrumRect.getX(), y - 10, 20, 20, juce::Justification::left);
		}

		g.setColour(juce::Colours::hotpink);

		// 8 probes for changes in amplitude
		for (int i = 0; i < 8; ++i)
		{
			if (spectrumData.data()[i * (spectrumData.size() / 8)] > 0.1f ||
				spectrumData.data()[i * (spectrumData.size() / 8)] < -10.1f)
			{
				for (size_t i = 0; i < prevFrames.size(); ++i)
				{
					float opacity = 0.1f + 0.9f * (i / (float)prevFrames.size());
					g.setColour(juce::Colours::hotpink.withAlpha(opacity));

					plot(prevFrames[i].data(), prevFrames[i].size() / 4, g, spectrumRect);
				}
			}
		}
	}

	void resized() override {}

private:
	AudioBufferQueue<SampleType>& audioBufferQueue;
	std::array<SampleType, AudioBufferQueue<SampleType>::bufferSize> buffer;
	std::array<std::array<SampleType, 2 * AudioBufferQueue<SampleType>::bufferSize>, NUM_PREV_FRAMES> prevFrames;
	int currentFrame = 0;
	juce::dsp::FFT fft{ AudioBufferQueue<SampleType>::order };
	juce::dsp::WindowingFunction<SampleType> windowFun{ (size_t)fft.getSize(),
		                             juce::dsp::WindowingFunction<SampleType>::hann };
	std::array<SampleType, 2 * AudioBufferQueue<SampleType>::bufferSize> spectrumData;
	const char* xLabels[9] { "64", "128", "256", "512", "1k", "2k", "4k", "8k", "16k"};
	const char* ampLabels[7] { " 18", " 12", "  6", "  0", " -6", "-12", "-18" };

	void timerCallback() override 
	{
		audioBufferQueue.pop(buffer.data());
		FloatVectorOperations::copy(spectrumData.data(), buffer.data(), (int)buffer.size());
		jassert(spectrumData.size() == 2 * (size_t)fft.getSize());
		windowFun.multiplyWithWindowingTable(spectrumData.data(), (size_t)fft.getSize());
		fft.performFrequencyOnlyForwardTransform(spectrumData.data());
		static constexpr auto mindB = SampleType(-160);
		static constexpr auto maxdB = SampleType(0);
		for (auto& s : spectrumData)
			s = jmap(jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(s) - juce::Decibels::gainToDecibels(SampleType((size_t)fft.getSize()))), mindB, maxdB, SampleType(0), SampleType(1));
		

		prevFrames[currentFrame++] = spectrumData;
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
		auto left = rect.getX();

		SampleType minFreq = 20;
		SampleType maxFreq = 20000;

		SampleType logScaleFactor = w / (log10(maxFreq) - log10(minFreq));

		for (size_t i = 1; i < numSamples; ++i)
		{
			SampleType freq = jmap(SampleType(i), SampleType(0), SampleType(numSamples), minFreq, maxFreq);
			SampleType prevFreq = jmap(SampleType(i - 1), SampleType(0), SampleType(numSamples), minFreq, maxFreq);

			SampleType xPos = (log10(freq) - log10(minFreq)) * logScaleFactor;
			SampleType prevXPos = (log10(prevFreq) - log10(minFreq)) * logScaleFactor;

			g.drawLine({ left + prevXPos, center - gain * data[i - 1], left + xPos, center - gain * data[i] });
		}
	}
};

template <typename SampleType>
class SpectrumScopeDataCollector 
{
public:
	SpectrumScopeDataCollector(AudioBufferQueue<SampleType>& queueToUse)
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
	enum class State { waitingForTrigger, collecting } state { State::waitingForTrigger };
};
