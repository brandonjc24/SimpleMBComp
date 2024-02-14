/*
  ==============================================================================

    CompressorBandControls.h
    Created: 14 Feb 2024 8:09:07am
    Author:  brand

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "RotarySliderWithLabels.h"


struct CompressorBandControls : juce::Component, juce::Button::Listener
{
    CompressorBandControls(juce::AudioProcessorValueTreeState& apvts);
    ~CompressorBandControls() override;
    void resized() override;
    void paint(juce::Graphics&) override;

    void buttonClicked(juce::Button* button) override;
private:
    juce::AudioProcessorValueTreeState& apvts;

    RotarySliderWithLabels attackSlider, releaseSlider, thresholdSlider;
    RatioSlider ratioSlider;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> attackSliderAttachment, releaseSliderAttachment, thresholdSliderAttachment, ratioSliderAttachment;

    using BtnAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<BtnAttachment> bypassButtonAttachment, soloButtonAttachment, muteButtonAttachment;

    juce::Component::SafePointer<CompressorBandControls> safePtr{ this };
    void updateAttachments();
    void updateSliderEnablements();
    void updateSoloMuteBypassToggleStates(juce::Button& clickedButton);
    void updateActiveFillBandColours(juce::Button& clickedButton);
    void resetActiveBandColours();

    static void refreshBandButtonColours(juce::Button& band, juce::Button& colourSource);
    void updateBandSelectButtonStates();

    juce::ToggleButton* activeBand = &lowBand;

    juce::ToggleButton bypassButton, soloButton, muteButton, lowBand, midBand, highBand;
};
