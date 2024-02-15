/*
  ==============================================================================

    PathProducer.h
    Created: 14 Feb 2024 12:22:51pm
    Author:  brand

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../GUI/FFTDataGenerator.h"
#include "../GUI/Utilities.h"
#include "../GUI/AnalyzerPathGenerator.h"
#include "../PluginProcessor.h"

struct PathProducer
{
    PathProducer(SingleChannelSampleFifo<SimpleMBCompAudioProcessor::BlockType>& scsf) :
        leftChannelFifo(&scsf)
    {
        leftChannelFFTDataGenerator.changeOrder(FFTOrder::order8192);
        monoBuffer.setSize(1, leftChannelFFTDataGenerator.getFFTSize());
    }
    void process(juce::Rectangle<float> fftBounds, double sampleRate);
    juce::Path getPath() { return leftChannelFFTPath; }

    void updateNEgativeInfinity(float nf) { negativeInf = nf; }
private:
    SingleChannelSampleFifo<SimpleMBCompAudioProcessor::BlockType>* leftChannelFifo;

    juce::AudioBuffer<float> monoBuffer;

    FFTDataGenerator<std::vector<float>> leftChannelFFTDataGenerator;

    AnalyzerPathGenerator<juce::Path> pathProducer;

    juce::Path leftChannelFFTPath;
    float negativeInf{ -48.f };
};
