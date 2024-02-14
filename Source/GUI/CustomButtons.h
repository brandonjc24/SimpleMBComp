/*
  ==============================================================================

    CustomButtons.h
    Created: 14 Feb 2024 7:56:21am
    Author:  brand

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct PowerButton : juce::ToggleButton { };

struct AnalyzerButton : juce::ToggleButton
{
    void resized() override;

    juce::Path randomPath;
};