/*
  ==============================================================================

    CompressorBand.h
    Created: 14 Feb 2024 8:15:03am
    Author:  brand

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../GUI/Utilities.h"

struct CompressorBand
{
    juce::AudioParameterFloat* attack{ nullptr };
    juce::AudioParameterFloat* release{ nullptr };
    juce::AudioParameterFloat* threshold{ nullptr };
    juce::AudioParameterChoice* ratio{ nullptr };
    juce::AudioParameterBool* bypassed{ nullptr };
    juce::AudioParameterBool* mute{ nullptr };
    juce::AudioParameterBool* solo{ nullptr };

    void prepare(const juce::dsp::ProcessSpec& spec);

    void updateCompressorSettings();

    void process(juce::AudioBuffer<float>& buffer);

    float getRMSOutputLeveldB() const { return rmsOutputLeveldB; }
    float getRMSInputLeveldB() const { return rmsInputLeveldB; }
private:
    juce::dsp::Compressor<float> compressor;

    std::atomic<float> rmsInputLeveldB {NEGATIVE_INFINITY};
    std::atomic<float> rmsOutputLeveldB {NEGATIVE_INFINITY};

    template<typename T>
    float computeRMSLevel(const T& buffer)
    {
        int numChannels = static_cast<int>(buffer.getNumChannels());
        int numSamples = static_cast<int>(buffer.getNumSamples());
        auto rms = 0.f;

        for (int chan = 0; chan < numChannels; ++chan)
            rms += buffer.getRMSLevel(chan, 0, numSamples);

        rms /= static_cast<float>(numChannels);
        return rms;

    }
};
