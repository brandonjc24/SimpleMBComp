/*
  ==============================================================================

    Utilities.h
    Created: 14 Feb 2024 8:03:28am
    Author:  brand

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#define MIN_FREQUENCY 20.F
#define MAX_FREQUENCY 20000.f

#define NEGATIVE_INFINITY -72.f
#define MAX_DECIBELS 12.f

#define MIN_THRESHOLD -60.f

enum FFTOrder
{
    order2048 = 11,
    order4096 = 12,
    order8192 = 13
};

enum Channel
{
    Right, //effectively 0
    Left //effectively 1
};

template<
    typename Attachment,
    typename APVTS,
    typename Params,
    typename ParamName,
    typename SliderType>
void makeAttachment(std::unique_ptr<Attachment>& attachment, APVTS& apvts, const Params& params, const ParamName& name, SliderType& slider)
{
    attachment = std::make_unique<Attachment>(apvts, params.at(name), slider);
}

template<
    typename APVTS,
    typename Params,
    typename Name>
juce::RangedAudioParameter& getParam(APVTS& apvts, const Params& params, const Name& name)
{
    auto param = apvts.getParameter(params.at(name));
    jassert(param != nullptr);
    return *param;
}

juce::String getValString(const juce::RangedAudioParameter& param, bool getLow, juce::String suffix);


template<typename T>
bool truncateKValue(T& value)
{
    if (value > static_cast<T>(999))
    {
        value /= static_cast<T>(1000);
        return true;
    }
    return false;
}

template<
    typename Labels,
    typename ParamType,
    typename SuffixType>
void addLabelPairs(Labels& labels, const ParamType& param, const SuffixType& suffix)
{
    labels.clear();
    labels.add({ 0.f, getValString(param, true, suffix) });
    labels.add({ 1.f, getValString(param, false, suffix) });
}

juce::Rectangle<int> drawModuleBackground(juce::Graphics& g, juce::Rectangle<int> bounds);