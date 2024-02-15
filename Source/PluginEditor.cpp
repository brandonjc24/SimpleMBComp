/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "GUI/PathProducer.h"
#include "DSP/CompressorBand.h"
#include "DSP/Params.h"

ControlBar::ControlBar()
{
    analyzerButton.setToggleState(true, juce::NotificationType::dontSendNotification);
    addAndMakeVisible(analyzerButton);
    addAndMakeVisible(globalBypassButton);
}

void ControlBar::resized()
{
    auto bounds = getLocalBounds();
    analyzerButton.setBounds(bounds.removeFromLeft(50).withTrimmedTop(4).withTrimmedBottom(4));
    globalBypassButton.setBounds(bounds.removeFromRight(60).withTrimmedTop(2).withTrimmedBottom(2));
}
//==============================================================================
SimpleMBCompAudioProcessorEditor::SimpleMBCompAudioProcessorEditor (SimpleMBCompAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    setLookAndFeel(&lnf);
    controlBar.analyzerButton.onClick = [this]()
        {
            auto shouldBeOn = controlBar.analyzerButton.getToggleState();
            analyzer.toggleAnalysisEnablement(shouldBeOn);
        };

    controlBar.globalBypassButton.onClick = [this]() {
        toggleGlobalBypassState();
        };

    addAndMakeVisible(controlBar);
    addAndMakeVisible(analyzer);
    addAndMakeVisible(globalControls);
    addAndMakeVisible(bandControls);

    setSize (550, 550);

    startTimerHz(60);
}

SimpleMBCompAudioProcessorEditor::~SimpleMBCompAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void SimpleMBCompAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void SimpleMBCompAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();

    controlBar.setBounds(bounds.removeFromTop(25));
    globalControls.setBounds(bounds.removeFromBottom(140));
    analyzer.setBounds(bounds.removeFromTop(230));
    bandControls.setBounds(bounds);
}

void SimpleMBCompAudioProcessorEditor::timerCallback()
{
    std::vector<float> values
    {
        audioProcessor.lowBandComp.getRMSInputLeveldB(),
        audioProcessor.lowBandComp.getRMSOutputLeveldB(),
        audioProcessor.midBandComp.getRMSInputLeveldB(),
        audioProcessor.midBandComp.getRMSOutputLeveldB(),
        audioProcessor.highBandComp.getRMSInputLeveldB(),
        audioProcessor.highBandComp.getRMSOutputLeveldB()
    };

    analyzer.update(values);

    updateGlobalBypassButton();
}
void SimpleMBCompAudioProcessorEditor::updateGlobalBypassButton()
{
    auto params = getBypassParams();
    bool allBandsAreBypassed = std::all_of(params.begin(), params.end(), [](const auto& param) { return param->get(); });

    controlBar.globalBypassButton.setToggleState(allBandsAreBypassed, juce::NotificationType::dontSendNotification);
}

void SimpleMBCompAudioProcessorEditor::toggleGlobalBypassState()
{
    auto shouldEnableEverything = !controlBar.globalBypassButton.getToggleState();
    auto params = getBypassParams();

    auto bypassParamHelper = [](auto* param, bool shouldBeBypassed) {
            param->beginChangeGesture();
            param->setValueNotifyingHost(shouldBeBypassed ? 1.f : 0.f);
            param->endChangeGesture();
        };

    for (auto* param : params)
    {
        bypassParamHelper(param, !shouldEnableEverything);
    }
    bandControls.toggleAllBands(!shouldEnableEverything);
}

std::array<juce::AudioParameterBool*, 3> SimpleMBCompAudioProcessorEditor::getBypassParams()
{
    using namespace Params;
    using namespace juce;
    const auto& params = Params::GetParams();
    auto& apvts = audioProcessor.apvts;

    auto boolHelper = [&apvts, &params](const auto& paramName)
        { 
            auto param = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(params.at(paramName))); 
            return param;
        };

    auto* lowBypassParam = boolHelper(Names::Bypass_Low_Band);
    auto* midBypassParam = boolHelper(Names::Bypass_Mid_Band);
    auto* highBypassParam = boolHelper(Names::Bypass_High_Band);

    return
    {
        lowBypassParam,
        midBypassParam,
        highBypassParam
    };
}