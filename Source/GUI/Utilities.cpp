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

void drawModuleBackground(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    using namespace juce;
    g.setColour(Colours::darkblue);
    g.fillAll();

    auto localBounds = bounds;
    bounds.reduce(3, 3);
    g.setColour(Colours::slategrey);
    g.fillRect(bounds.toFloat());

    g.drawRect(localBounds);
}