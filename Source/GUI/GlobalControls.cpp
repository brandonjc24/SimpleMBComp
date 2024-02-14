/*
  ==============================================================================

    GlobalControls.cpp
    Created: 14 Feb 2024 8:11:14am
    Author:  brand

  ==============================================================================
*/

#include <JuceHeader.h>
#include "GlobalControls.h"
#include "../DSP/Params.h"
#include "Utilities.h"

GlobalControls::GlobalControls(juce::AudioProcessorValueTreeState& apvts)
{
    using namespace Params;
    const auto& params = GetParams();

    auto getParamHelper = [&params, &apvts](const auto& name) -> auto&
        {
            return getParam(apvts, params, name);
        };

    auto& gainInParam = getParamHelper(Names::Gain_In);
    auto& lowMidParam = getParamHelper(Names::Low_Mid_Crossover_Freq);
    auto& midHighParam = getParamHelper(Names::Mid_High_Crossover_Freq);
    auto& gainOutParam = getParamHelper(Names::Gain_Out);

    inGainSlider = std::make_unique<RSWL>(&gainInParam, " dB", "In");
    lowMinXoverSlider = std::make_unique<RSWL>(&lowMidParam, " Hz", "L->M");
    midHighXoverSlider = std::make_unique<RSWL>(&midHighParam, " Hz", "M->H");
    outGainSlider = std::make_unique<RSWL>(&gainOutParam, " dB", "Out");

    auto makeAttachmentHelper = [&params, &apvts](auto& attachment, const auto& name, auto& slider)
        {
            makeAttachment(attachment, apvts, params, name, slider);
        };

    makeAttachmentHelper(inGainSliderAttachment, Names::Gain_In, *inGainSlider);
    makeAttachmentHelper(lowMidXoverSliderAttachment, Names::Low_Mid_Crossover_Freq, *lowMinXoverSlider);
    makeAttachmentHelper(midHighXoverSliderAttachment, Names::Mid_High_Crossover_Freq, *midHighXoverSlider);
    makeAttachmentHelper(outGainSliderAttachment, Names::Gain_Out, *outGainSlider);

    addLabelPairs(inGainSlider->labels, gainInParam, " dB");
    addLabelPairs(lowMinXoverSlider->labels, lowMidParam, " Hz");
    addLabelPairs(midHighXoverSlider->labels, midHighParam, " Hz");
    addLabelPairs(outGainSlider->labels, gainOutParam, " dB");

    addAndMakeVisible(*inGainSlider);
    addAndMakeVisible(*lowMinXoverSlider);
    addAndMakeVisible(*midHighXoverSlider);
    addAndMakeVisible(*outGainSlider);
}

void GlobalControls::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    drawModuleBackground(g, bounds);
}

void GlobalControls::resized()
{
    using namespace juce;
    auto bounds = getLocalBounds().reduced(5);

    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;

    auto spacer = FlexItem().withWidth(4);
    auto endCap = FlexItem().withWidth(6);

    flexBox.items.add(endCap);
    flexBox.items.add(FlexItem(*inGainSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*lowMinXoverSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*midHighXoverSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*outGainSlider).withFlex(1.f));
    flexBox.items.add(endCap);


    flexBox.performLayout(bounds);
}
