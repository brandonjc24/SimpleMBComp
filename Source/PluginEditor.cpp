/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "GUI/PathProducer.h"
#include "DSP/CompressorBand.h"

//==============================================================================
SimpleMBCompAudioProcessorEditor::SimpleMBCompAudioProcessorEditor (SimpleMBCompAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    setLookAndFeel(&lnf);
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
}
