#pragma once

template <typename SampleType>
class AudioBufferQueue
{
public:

    void setBufferSize(size_t newSize)
    {
        bufferSize = newSize;
        buffers.resize(numBuffers);
        for (auto& b : buffers)
            b.resize(bufferSize);
        abstractFifo.setTotalSize((int)numBuffers);
    }

    size_t getBufferSize() const noexcept { return bufferSize; }

    bool push(const SampleType* dataToPush, size_t numSamples) noexcept
    {
        jassert(numSamples == bufferSize);

        int start1, size1, start2, size2;
        abstractFifo.prepareToWrite(1, start1, size1, start2, size2);

        if (size1 == 0)
            return false; // queue full, nothing written

        auto* dest = buffers[(size_t)start1].data();

        FloatVectorOperations::copy(dest, dataToPush, (int)numSamples);

        // optional: clear the tail if numSamples < bufferSize
        if (numSamples < bufferSize)
            FloatVectorOperations::fill(dest + numSamples, SampleType(0),
                                        (int)(bufferSize - numSamples));

        abstractFifo.finishedWrite(size1);
        return true;
    }

    bool pop(SampleType* outputBuffer) noexcept
    {
        int start1, size1, start2, size2;
        abstractFifo.prepareToRead(1, start1, size1, start2, size2);

        if (size1 == 0)
            return false; // queue empty, nothing read

        FloatVectorOperations::copy(outputBuffer,
            buffers[(size_t)start1].data(),
            (int)bufferSize);

        abstractFifo.finishedRead(size1);
        return true;
    }

private:
    juce::AbstractFifo abstractFifo { (int)numBuffers };
    size_t bufferSize = 0;
    
    std::vector<std::vector<SampleType>> buffers;
    static constexpr size_t numBuffers = 5;
};
