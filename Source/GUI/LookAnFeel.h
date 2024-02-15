/*
  ==============================================================================

    LookAnFeel.h
    Created: 14 Feb 2024 7:49:53am
    Author:  brand

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define USE_LIVE_CONTANT false

#if USE_LIVE_CONTANT
#define colorHelper(c) JUCE_LIVE_CONSTANT(c)
#else
#define colorHelper(c) c;
#endif

namespace ColorScheme
{
inline juce::Colour getSliderBorderColor()
{
    return colorHelper(juce::Colours::black);
}
inline juce::Colour getSliderFillColor()
{
    return colorHelper(juce::Colour(0xff256745));
}
inline juce::Colour getSliderDisabledColor()
{
    return colorHelper(juce::Colours::darkslategrey);
}
}



struct LookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider(juce::Graphics&,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider&) override;

    void drawToggleButton(juce::Graphics& g,
        juce::ToggleButton& toggleButton,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;
};
