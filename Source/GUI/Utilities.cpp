/*
  ==============================================================================

    Utilities.cpp
    Created: 14 Feb 2024 8:03:28am
    Author:  brand

  ==============================================================================
*/

#include "Utilities.h"

juce::String getValString(const juce::RangedAudioParameter& param, bool getLow, juce::String suffix)
{
    juce::String str;

    auto val = getLow ? param.getNormalisableRange().start : param.getNormalisableRange().end;

    bool useK = truncateKValue(val);
    str << val;
    if (useK)
        str << "k";

    str << suffix;
    return str;
}

juce::Rectangle<int> drawModuleBackground(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    using namespace juce;
    g.setColour(Colours::darkblue); // module outline
    g.fillAll();

    auto localBounds = bounds;
    bounds.reduce(3, 3);
    g.setColour(Colours::darkgrey); // background color
    g.fillRect(bounds.toFloat());

    g.drawRect(localBounds);

    return bounds;
}