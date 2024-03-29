/*
  ==============================================================================

    PathProducer.cpp
    Created: 14 Feb 2024 12:22:52pm
    Author:  brand

  ==============================================================================
*/

#include "PathProducer.h"
#include <JuceHeader.h>

void PathProducer::process(juce::Rectangle<float> fftBounds, double sampleRate)
{
    juce::AudioBuffer<float> tempIncomingBuffer;
    while (leftChannelFifo->getNumCompleteBuffersAvailable() > 0)
    {
        if (leftChannelFifo->getAudioBuffer(tempIncomingBuffer))
        {
            auto size = tempIncomingBuffer.getNumSamples();

            jassert(size <= monoBuffer.getNumSamples());
            size = juce::jmin(size, monoBuffer.getNumSamples());

            auto writePointer = monoBuffer.getWritePointer(0, 0);
            auto readPointer = monoBuffer.getReadPointer(0, size);

            std::copy(readPointer, readPointer + (monoBuffer.getNumSamples() - size), writePointer);

            juce::FloatVectorOperations::copy(monoBuffer.getWritePointer(0, monoBuffer.getNumSamples() - size),
                tempIncomingBuffer.getReadPointer(0, 0),
                size);

            leftChannelFFTDataGenerator.produceFFTDataForRendering(monoBuffer, negativeInf);
        }
    }

    const auto fftSize = leftChannelFFTDataGenerator.getFFTSize();
    const auto binWidth = sampleRate / double(fftSize);

    while (leftChannelFFTDataGenerator.getNumAvailableFFTDataBlocks() > 0)
    {
        std::vector<float> fftData;
        if (leftChannelFFTDataGenerator.getFFTData(fftData))
        {
            pathProducer.generatePath(fftData, fftBounds, fftSize, binWidth, negativeInf);
        }
    }

    while (pathProducer.getNumPathsAvailable() > 0)
    {
        pathProducer.getPath(leftChannelFFTPath);
    }
}
