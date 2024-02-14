/*
  ==============================================================================

    UtilityComponents.h
    Created: 14 Feb 2024 7:59:46am
    Author:  brand

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

struct Placeholder : juce::Component
{
    Placeholder();

    void paint(juce::Graphics& g) override;

    juce::Colour customColor;
};

struct RotarySlider : juce::Slider
{
    RotarySlider();
};