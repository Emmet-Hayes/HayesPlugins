#pragma once
#include <cmath>
#include "CustomLookAndFeel.h"

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
	SpectrumScopeComponent(AudioBufferQueue<SampleType>& queueToUse, CustomLookAndFeel& laf)
	:   audioBufferQueue(queueToUse)
    ,   lookAndFeel(laf)
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

		auto spectrumRect = juce::Rectangle<SampleType>{ SampleType(20), SampleType(0), w, h - 20};

		// draw grid
		g.setColour(juce::Colours::darkturquoise);
		g.setFont(lookAndFeel.getCommonMenuFont(scaleH));
		int numVerticalLines = 10;
		int numHorizontalLines = 8;

		for (int i = 1; i < numVerticalLines; ++i)
		{
			float x = jmap<float>(static_cast<float>(i), 0.0f, static_cast<float>(numVerticalLines), spectrumRect.getX(), spectrumRect.getRight());
			g.drawLine(x, spectrumRect.getY(), x, spectrumRect.getBottom());
			g.drawText(xLabels[i - 1], 
				       static_cast<int>(x - (5 * scaleW)), 
				       static_cast<int>(spectrumRect.getBottom() - (5 * scaleH)), 
				       static_cast<int>(30 * scaleW), 
				       static_cast<int>(20 * scaleH), juce::Justification::left);
		}

		for (int i = 1; i < numHorizontalLines; ++i)
		{
			float y = jmap<float>(static_cast<float>(i), 0.0f, static_cast<float>(numHorizontalLines), spectrumRect.getY(), spectrumRect.getBottom());
			g.drawLine(spectrumRect.getX(), y, spectrumRect.getRight(), y);
			g.drawText(ampLabels[i - 1], 
				       0, 
				       static_cast<int>(y - (10 * scaleH)), 
				       static_cast<int>(30 * scaleW), 
				       static_cast<int>(20 * scaleH), juce::Justification::left);
		}

		// 8 probes for changes in amplitude, this is more efficient than scanning the whole buffer for updates.
		for (int j = 0; j < 8; ++j)
		{
			if (spectrumData.data()[j * (spectrumData.size() / 8)] > 0.1f ||
				spectrumData.data()[j * (spectrumData.size() / 8)] < -10.1f)
			{
				for (size_t i = 0; i < prevFrames.size(); ++i)
				{
					float opacity = 0.1f + 0.9f * (i / (float)prevFrames.size());
					
					g.setColour(juce::Colours::hotpink.withAlpha(opacity));
					juce::ColourGradient grad (juce::Colours::pink.withAlpha(opacity), 0.0f, 0.0f, 
						                       juce::Colours::blue.withAlpha(opacity), 0.0f, static_cast<float>(getLocalBounds().getHeight()), false);
                    g.setGradientFill(grad);

					plot(prevFrames[i].data(), prevFrames[i].size() / 4, g, spectrumRect);
				}
			}
		}
	}

	void setScale(float newScaleW, float newScaleH) { scaleW = newScaleW; scaleH = newScaleH; }

	void resized() override {}

private:
	CustomLookAndFeel& lookAndFeel;
	AudioBufferQueue<SampleType>& audioBufferQueue;
	std::array<SampleType, AudioBufferQueue<SampleType>::bufferSize> buffer;
	std::array<std::array<SampleType, 2 * AudioBufferQueue<SampleType>::bufferSize>, NUM_PREV_FRAMES> prevFrames;
	int currentFrame = 0;
	juce::dsp::FFT fft{ AudioBufferQueue<SampleType>::order };
	juce::dsp::WindowingFunction<SampleType> windowFun{ (size_t)fft.getSize(),
		                             juce::dsp::WindowingFunction<SampleType>::hann };
	std::array<SampleType, 2 * AudioBufferQueue<SampleType>::bufferSize> spectrumData;
	const char* xLabels[9] { "50", "100", "200", "400", "800", "1.6k", "3.2k", "6.4k", "13k"};
	const char* ampLabels[7] { " 15", " 10", "  5", "  0", " -5", "-10", "-15" };
	float scaleW = 1.0f;
	float scaleH = 1.0f;

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
		//auto right = rect.getRight();
		auto center = rect.getBottom() - offset;
		auto gain = h * scale;
		auto left = rect.getX();

		SampleType minFreq = 20;
		SampleType maxFreq = 20000;

		SampleType logScaleFactor = w / (log10(maxFreq) - log10(minFreq));

		// create a Path to represent the plot
		Path plotPath;

		// add the first point to the Path
		SampleType firstFreq = jmap(SampleType(0), SampleType(0), SampleType(numSamples), minFreq, maxFreq);
		SampleType firstXPos = (log10(firstFreq) - log10(minFreq)) * logScaleFactor;
		plotPath.startNewSubPath(left + firstXPos, center - gain * data[0]);

		for (size_t i = 1; i < numSamples; ++i)
		{
			SampleType freq = jmap(SampleType(i), SampleType(0), SampleType(numSamples), minFreq, maxFreq);
			SampleType xPos = (log10(freq) - log10(minFreq)) * logScaleFactor;

			// add each point to the Path
			plotPath.lineTo(left + xPos, center - gain * data[i]);
		}

		// close the Path at the bottom
		plotPath.lineTo(left + w, center);
		plotPath.lineTo(left, center);
		plotPath.closeSubPath();

		// fill the Path
		g.setOpacity(0.33f);
		g.fillPath(plotPath);
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
