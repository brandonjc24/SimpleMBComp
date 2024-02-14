/*
  ==============================================================================

    CompressorBandControls.cpp
    Created: 14 Feb 2024 8:09:07am
    Author:  brand

  ==============================================================================
*/

#include <JuceHeader.h>
#include "CompressorBandControls.h"
#include "Utilities.h"
#include "../DSP/Params.h"

CompressorBandControls::CompressorBandControls(juce::AudioProcessorValueTreeState& apv) :
    apvts(apv),
    attackSlider(nullptr, " ms", "X"),
    releaseSlider(nullptr, " ms", "R"),
    thresholdSlider(nullptr, " dB", "T"),
    ratioSlider(nullptr, " ")
{
    addAndMakeVisible(attackSlider);
    addAndMakeVisible(thresholdSlider);
    addAndMakeVisible(ratioSlider);
    addAndMakeVisible(releaseSlider);

    bypassButton.addListener(this);
    soloButton.addListener(this);
    muteButton.addListener(this);

    bypassButton.setName("X");
    bypassButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::darkslategrey);
    bypassButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::dimgrey);

    soloButton.setName("s");
    soloButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::darkmagenta);
    soloButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::dimgrey);

    muteButton.setName("m");
    muteButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::darkgreen);
    muteButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::dimgrey);

    addAndMakeVisible(bypassButton);
    addAndMakeVisible(soloButton);
    addAndMakeVisible(muteButton);

    lowBand.setName("L");
    lowBand.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::black);
    lowBand.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::dimgrey);

    midBand.setName("M");
    midBand.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::black);
    midBand.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::dimgrey);

    highBand.setName("^");
    highBand.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::black);
    highBand.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::dimgrey);
   
    
    auto buttonSwitcher = [safePtr = this->safePtr]()
        {
            if (auto* c = safePtr.getComponent())
            {
                c->updateAttachments();
            }
        };

    lowBand.onClick = buttonSwitcher;
    midBand.onClick = buttonSwitcher;
    highBand.onClick = buttonSwitcher;

    lowBand.setToggleState(true, juce::NotificationType::dontSendNotification);

    updateAttachments();
    updateSliderEnablements();
    updateBandSelectButtonStates();

    lowBand.setRadioGroupId(1);
    midBand.setRadioGroupId(1);
    highBand.setRadioGroupId(1);

    addAndMakeVisible(lowBand);
    addAndMakeVisible(midBand);
    addAndMakeVisible(highBand);
}

CompressorBandControls::~CompressorBandControls()
{
    bypassButton.removeListener(this);
    soloButton.removeListener(this);
    muteButton.removeListener(this);
}

void CompressorBandControls::resized()
{
    using namespace juce;
    auto bounds = getLocalBounds().reduced(5);

    auto createBandButtonControlBox = [](std::vector<Component*> comps)
        {
            FlexBox flexBox;
            flexBox.flexDirection = FlexBox::Direction::column;
            flexBox.flexWrap = FlexBox::Wrap::noWrap;

            auto spacer = FlexItem().withHeight(6);

            for (auto* comp : comps)
            {
                flexBox.items.add(spacer);
                flexBox.items.add(FlexItem(*comp).withFlex(1.f));
            }

            flexBox.items.add(spacer);

            return flexBox;
        };

    auto bandButtonControlBox = createBandButtonControlBox({ &bypassButton, &soloButton, &muteButton });
    auto bandSelectControlBox = createBandButtonControlBox({ &highBand, &midBand, &lowBand });


    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;

    auto spacer = FlexItem().withWidth(4);
    auto endCap = FlexItem().withWidth(6);


    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(bandSelectControlBox).withWidth(30));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(attackSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(releaseSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(thresholdSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(ratioSlider).withFlex(1.f));

    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(bandButtonControlBox).withWidth(30));
    flexBox.performLayout(bounds);
}

void CompressorBandControls::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    drawModuleBackground(g, bounds);
}

void CompressorBandControls::buttonClicked(juce::Button* button)
{
    updateSliderEnablements();
    updateSoloMuteBypassToggleStates(*button);
    updateActiveFillBandColours(*button);
}

void CompressorBandControls::updateActiveFillBandColours(juce::Button& clickedButton)
{
    jassert(activeBand != nullptr);
    DBG("button: " << clickedButton.getName());

    if (clickedButton.getToggleState() == false)
    {
        resetActiveBandColours();
    }
    else
    {
        refreshBandButtonColours(*activeBand, clickedButton);
    }
  
}

void CompressorBandControls::refreshBandButtonColours(juce::Button& band, juce::Button& colourSource)
{
    band.setColour(juce::TextButton::ColourIds::buttonOnColourId, colourSource.findColour(juce::TextButton::ColourIds::buttonOnColourId));
    band.setColour(juce::TextButton::ColourIds::buttonColourId,   colourSource.findColour(juce::TextButton::ColourIds::buttonOnColourId));
    band.repaint();
}

void CompressorBandControls::resetActiveBandColours()
{
    activeBand->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::black);
    activeBand->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::dimgrey);
    activeBand->repaint();
}

void CompressorBandControls::updateBandSelectButtonStates()
{
    using namespace Params;

    std::vector<std::array<Names, 3>> paramsToCheck
    {
        {Names::Solo_Low_Band,  Names::Mute_Low_Band,  Names::Bypass_Low_Band },
        {Names::Solo_Mid_Band,  Names::Mute_Mid_Band,  Names::Bypass_Mid_Band },
        {Names::Solo_High_Band, Names::Mute_High_Band, Names::Bypass_High_Band},
    };

    const auto& params = GetParams();

    auto paramHelper = [&params, this](const auto& name)
        {
            return dynamic_cast<juce::AudioParameterBool*>(&getParam(apvts, params, name));
        };

    for (size_t i = 0; i < paramsToCheck.size(); ++i)
    {
        auto& list = paramsToCheck[i];

        auto* bandButton = (i == 0) ? &lowBand :
                           (i == 1) ? &midBand :
                                      &highBand;

        if (auto* solo = paramHelper(list[0]);
            solo->get())
        {
            refreshBandButtonColours(*bandButton, soloButton);
        }
        else if (auto* mute = paramHelper(list[1]);
            mute->get())
        {
            refreshBandButtonColours(*bandButton, muteButton);
        }
        else if (auto* bypass = paramHelper(list[2]);
            bypass->get())
        {
            refreshBandButtonColours(*bandButton, bypassButton);
        }
    }

}

void CompressorBandControls::updateSliderEnablements()
{
    auto disabled = muteButton.getToggleState() || bypassButton.getToggleState();
    attackSlider.setEnabled(!disabled);
    releaseSlider.setEnabled(!disabled);
    thresholdSlider.setEnabled(!disabled);
    ratioSlider.setEnabled(!disabled);
}

void CompressorBandControls::updateSoloMuteBypassToggleStates(juce::Button& clickedButton)
{
    if (&clickedButton == &soloButton && soloButton.getToggleState())
    {
        bypassButton.setToggleState(false, juce::NotificationType::sendNotification);
        muteButton.setToggleState(false, juce::NotificationType::sendNotification);
    }
    else if (&clickedButton == &muteButton && muteButton.getToggleState())
    {
        bypassButton.setToggleState(false, juce::NotificationType::sendNotification);
        soloButton.setToggleState(false, juce::NotificationType::sendNotification);
    }
    else if (&clickedButton == &bypassButton && bypassButton.getToggleState())
    {
        soloButton.setToggleState(false, juce::NotificationType::sendNotification);
        muteButton.setToggleState(false, juce::NotificationType::sendNotification);
    }

}

void CompressorBandControls::updateAttachments()
{
    enum BandType
    {
        Low,
        Mid,
        High
    };

    BandType bandType = [this]()
        {
            if (lowBand.getToggleState())
                return BandType::Low;
            if (midBand.getToggleState())
                return BandType::Mid;

            return BandType::High;
        }();

        using namespace Params;
        std::vector<Names> names;

        switch (bandType)
        {
            case Low:
            {
                names = std::vector<Names>
                {
                    Names::Attack_Low_band,
                    Names::Release_Low_Band,
                    Names::Threshold_Low_Band,
                    Names::Ratio_Low_Band,
                    Names::Mute_Low_Band,
                    Names::Solo_Low_Band,
                    Names::Bypass_Low_Band
                };
                activeBand = &lowBand;
                break;
            }
            case Mid:
            {
                names = std::vector<Names>
                {
                    Names::Attack_Mid_band,
                    Names::Release_Mid_Band,
                    Names::Threshold_Mid_Band,
                    Names::Ratio_Mid_Band,
                    Names::Mute_Mid_Band,
                    Names::Solo_Mid_Band,
                    Names::Bypass_Mid_Band
                };
                activeBand = &midBand;
                break;
            }
            case High:
            {
                names = std::vector<Names>
                {
                    Names::Attack_High_band,
                    Names::Release_High_Band,
                    Names::Threshold_High_Band,
                    Names::Ratio_High_Band,
                    Names::Mute_High_Band,
                    Names::Solo_High_Band,
                    Names::Bypass_High_Band
                };
                activeBand = &highBand;
                break;
            }
        }

        DBG("Active Band: " << activeBand->getName());

        enum Pos
        {
            Attack,
            Release,
            Threshold,
            Ratio,
            Mute,
            Solo,
            Bypass
        };

        const auto& params = GetParams();

        auto getParamHelper = [&params, &apvts = this -> apvts, &names](const auto& pos) -> auto&
            {
                return getParam(apvts, params, names.at(pos));
            };


        attackSliderAttachment.reset();
        releaseSliderAttachment.reset();
        thresholdSliderAttachment.reset();
        ratioSliderAttachment.reset();
        bypassButtonAttachment.reset();
        soloButtonAttachment.reset();
        muteButtonAttachment.reset();

        auto& attackParam = getParamHelper(Pos::Attack);
        addLabelPairs(attackSlider.labels, attackParam, "ms");
        attackSlider.changeParam(&attackParam);

        auto& releaseParam = getParamHelper(Pos::Release);
        addLabelPairs(releaseSlider.labels, releaseParam, "ms");
        releaseSlider.changeParam(&releaseParam);

        auto& threshParam = getParamHelper(Pos::Threshold);
        addLabelPairs(thresholdSlider.labels, threshParam, "dB");
        thresholdSlider.changeParam(&threshParam);

        auto& ratioParamRap = getParamHelper(Pos::Ratio);
        ratioSlider.labels.clear();
        ratioSlider.labels.add({ 0.f, "1:1" });
        auto ratioParam = dynamic_cast<juce::AudioParameterChoice*>(&ratioParamRap);
        ratioSlider.labels.add({ 1.0f,
            juce::String(ratioParam->choices.getReference(ratioParam->choices.size() - 1).getIntValue()) + ":1" });
        ratioSlider.changeParam(ratioParam);

        auto makeAttachmentHelper = [&params, &apvts = this->apvts](auto& attachment, const auto& name, auto& slider)
            {
                makeAttachment(attachment, apvts, params, name, slider);
            };

        makeAttachmentHelper(attackSliderAttachment, names[Pos::Attack], attackSlider);
        makeAttachmentHelper(releaseSliderAttachment, names[Pos::Release], releaseSlider);
        makeAttachmentHelper(thresholdSliderAttachment, names[Pos::Threshold], thresholdSlider);
        makeAttachmentHelper(ratioSliderAttachment, names[Pos::Ratio], ratioSlider);
        makeAttachmentHelper(bypassButtonAttachment, names[Pos::Bypass], bypassButton);
        makeAttachmentHelper(soloButtonAttachment, names[Pos::Solo], soloButton);
        makeAttachmentHelper(muteButtonAttachment, names[Pos::Mute], muteButton);;

}
