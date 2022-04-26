/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

template<typename T>
bool truncateKValue(T& value)
{
    if (value > static_cast<T>(999))
    {
        value /= static_cast<T>(1000);
        return true;
    }
    return false;
}

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

void LookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x, int y, int width, int height,
    float sliderPosProportional,
    float rotaryStartAngle,
    float rotaryEndAngle,
    juce::Slider& slider)
{
    using namespace juce;

    auto bounds = Rectangle<float>(x, y, width, height);

    g.setColour(Colours::slategrey);
    g.fillEllipse(bounds);
    g.setColour(Colours::black);
    g.drawEllipse(bounds, 2.f);

    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto center = bounds.getCentre();
        Path p;

        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setTop(bounds.getY());
        r.setBottom(center.getY() - rswl->getTextHeight() * 1.5);

        p.addRoundedRectangle(r, 1.f);
        jassert(rotaryStartAngle < rotaryEndAngle);

        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

        g.fillPath(p);

        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);

        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(center);

        g.setColour(Colours::slategrey);
        g.fillRect(r);

        g.setColour(Colours::black);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }

}

void LookAndFeel::drawToggleButton(juce::Graphics& g,
    juce::ToggleButton& toggleButton,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown)
{
    using namespace juce;

    if (auto* pb = dynamic_cast<PowerButton*>(&toggleButton))
    {
        Path powerButton;

        auto bounds = toggleButton.getLocalBounds();

        auto size = jmin(bounds.getWidth(), bounds.getHeight()) - 6;
        auto r = bounds.withSizeKeepingCentre(size, size).toFloat();

        float ang = 30.f; //30.f;

        size -= 6;

        powerButton.addCentredArc(r.getCentreX(),
            r.getCentreY(),
            size * 0.5,
            size * 0.5,
            0.f,
            degreesToRadians(ang),
            degreesToRadians(360.f - ang),
            true);

        powerButton.startNewSubPath(r.getCentreX(), r.getY());
        powerButton.lineTo(r.getCentre());

        PathStrokeType pst(2.f, PathStrokeType::JointStyle::curved);

        auto color = toggleButton.getToggleState() ? Colours::dimgrey : Colour(0u, 172u, 1u);

        g.setColour(color);
        g.strokePath(powerButton, pst);
        g.drawEllipse(r, 2);
    }
    else if (auto* analyzerButton = dynamic_cast<AnalyzerButton*>(&toggleButton))
    {
        auto color = !toggleButton.getToggleState() ? Colours::dimgrey : Colour(0u, 172u, 1u);

        g.setColour(color);

        auto bounds = toggleButton.getLocalBounds();
        g.drawRect(bounds);

        g.strokePath(analyzerButton->randomPath, PathStrokeType(1.f));
    }
    else
    {
        auto bounds = toggleButton.getLocalBounds().reduced(2);

        auto buttonIsOn = toggleButton.getToggleState();

        const int cornerSize = 4;
        g.setColour(buttonIsOn ? juce::Colours::black : juce::Colours::dimgrey);
        g.fillRect(bounds.toFloat());

        g.setColour(buttonIsOn ? juce::Colours::white : juce::Colours::black);
        g.drawRect(bounds.toFloat(), 1);
        g.drawFittedText(toggleButton.getName(), bounds, juce::Justification::centred, 2);

    }
}

void RotarySliderWithLabels::paint(juce::Graphics& g)
{
    using namespace juce;

    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f + 360.f);

    auto range = getRange();
    auto sliderBounds = getSliderBouds();
    
    auto bounds = getLocalBounds();

    g.setColour(Colours::white);
    g.drawFittedText(getName(), bounds.removeFromTop(getTextBoxHeight()+2), Justification::centredBottom, 1);

    getLookAndFeel().drawRotarySlider(g,
        sliderBounds.getX(),
        sliderBounds.getY(),
        sliderBounds.getWidth(),
        sliderBounds.getHeight(),
        jmap(getValue(), range.getStart(), range.getEnd(), 0.0, 1.0),
        startAng,
        endAng,
        *this);

    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5f;

    g.setColour(Colours::white);
    g.setFont(getTextHeight());

    auto numChoices = labels.size();
    for (int i = 0; i < numChoices; ++i)
    {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        auto angle = jmap(pos, 0.f, 1.f, startAng, endAng);

        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, angle);

        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(g.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setBottom(r.getY() + getTextHeight() * 3);

        g.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBouds() const
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(getTextHeight()*2);

    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    size -= 1.5 * getTextHeight();

    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(bounds.getY());
    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if (auto* choiseParam = dynamic_cast<juce::AudioParameterChoice*>(param))
        return choiseParam->getCurrentChoiceName();

    juce::String str;
    bool addK = false;

    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        float val = getValue();

        addK = truncateKValue(val);

        str = juce::String(val, (addK ? 2 : 0));
    }
    else
    {
        jassertfalse;
    }

    if (suffix.isNotEmpty())
    {
        str << " ";
        if (addK)
            str << "k";

        str << suffix;
    }

    return str;
}

void RotarySliderWithLabels::changeParam(juce::RangedAudioParameter *p)
{
    param = p;
    repaint();
}
//==============================================================================
juce::String RatioSlider::getDisplayString() const
{
    auto choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param);

    auto currentChoice = choiceParam->getCurrentChoiceName();

    if (currentChoice.contains(".0"))
        currentChoice = currentChoice.substring(0, currentChoice.indexOf("."));

    currentChoice << ":1";

    return currentChoice;
}
//==============================================================================
Placeholder::Placeholder()
{
    juce::Random r;
    customColor = juce::Colour(r.nextInt(255), r.nextInt(255), r.nextInt(255));
}
//==============================================================================
CompressorBandControls::CompressorBandControls(juce::AudioProcessorValueTreeState& apv):
    apvts(apv),
    attackSlider(nullptr, " ms", "X"),
    releaseSlider(nullptr, " ms", "R"),
    thresholdSlider(nullptr, " dB", "T"),
    ratioSlider(nullptr, " ")
{
    using namespace Params;
    const auto& params = GetParams();

    auto getParamHelper = [&params, &apvts = this->apvts](const auto& name) -> auto&
    {
        return getParam(apvts, params, name);
    };

    attackSlider.changeParam(&getParamHelper(Names::Attack_Mid_band));
    releaseSlider.changeParam(&getParamHelper(Names::Release_Mid_Band));
    thresholdSlider.changeParam(&getParamHelper(Names::Threshold_Mid_Band));
    ratioSlider.changeParam(&getParamHelper(Names::Ratio_Mid_Band));

    addLabelPairs(attackSlider.labels, getParamHelper(Names::Attack_Mid_band), " ms");
    addLabelPairs(releaseSlider.labels, getParamHelper(Names::Release_Mid_Band), " ms");
    addLabelPairs(thresholdSlider.labels, getParamHelper(Names::Threshold_Mid_Band), " dB");
    
    ratioSlider.labels.add({ 0.f, "1:1" });
    auto ratioParam = dynamic_cast<juce::AudioParameterChoice*>(&getParamHelper(Names::Ratio_Mid_Band));
    ratioSlider.labels.add({ 1.f, juce::String(ratioParam->choices.getReference(ratioParam->choices.size() - 1).getIntValue()) + ":1"});

    auto makeAttachmentHelper = [&params, &apvts = this->apvts](auto& attachment, const auto& name, auto& slider)
    {
        makeAttachment(attachment, apvts, params, name, slider);
    };

    makeAttachmentHelper(attackSliderAttachment, Names::Attack_Mid_band, attackSlider);
    makeAttachmentHelper(releaseSliderAttachment, Names::Release_Mid_Band, releaseSlider);
    makeAttachmentHelper(thresholdSliderAttachment, Names::Threshold_Mid_Band, thresholdSlider);
    makeAttachmentHelper(ratioSliderAttachment, Names::Ratio_Mid_Band, ratioSlider);

    addAndMakeVisible(attackSlider);
    addAndMakeVisible(thresholdSlider);
    addAndMakeVisible(ratioSlider);
    addAndMakeVisible(releaseSlider);

    bypassButton.setName("X");
    soloButton.setName("s");
    muteButton.setName("m");

    addAndMakeVisible(bypassButton);
    addAndMakeVisible(soloButton);
    addAndMakeVisible(muteButton);

    makeAttachmentHelper(bypassButtonAttachment, Names::Bypass_Mid_Band, bypassButton);
    makeAttachmentHelper(soloButtonAttachment, Names::Solo_Low_Band, soloButton);
    makeAttachmentHelper(muteButtonAttachment, Names::Mute_Mid_Band, muteButton);

    lowBand.setName("L");
    midBand.setName("M");
    highBand.setName("^");

    lowBand.setRadioGroupId(1);
    midBand.setRadioGroupId(1);
    highBand.setRadioGroupId(1);

    addAndMakeVisible(lowBand);
    addAndMakeVisible(midBand);
    addAndMakeVisible(highBand);
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

    auto bandButtonControlBox = createBandButtonControlBox({&bypassButton, &soloButton, &muteButton});
    auto bandSelectControlBox = createBandButtonControlBox({&highBand, &midBand, &lowBand });


    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;

    auto spacer = FlexItem().withWidth(4);
    auto endCap = FlexItem().withWidth(6);

    //flexBox.items.add(endCap);
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
    //flexBox.items.add(endCap);
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(bandButtonControlBox).withWidth(30));
    flexBox.performLayout(bounds);
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

void CompressorBandControls::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    drawModuleBackground(g, bounds);
}

//==============================================================================
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

//==============================================================================
SimpleMBCompAudioProcessorEditor::SimpleMBCompAudioProcessorEditor (SimpleMBCompAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    setLookAndFeel(&lnf);
    //addAndMakeVisible(controlBar);
    //addAndMakeVisible(analyzer);
    addAndMakeVisible(globalControls);
    addAndMakeVisible(bandControls);

    setSize (550, 550);
}

SimpleMBCompAudioProcessorEditor::~SimpleMBCompAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void SimpleMBCompAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    //g.setColour (juce::Colours::white);
    //g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);

    g.fillAll(juce::Colours::black);
}

void SimpleMBCompAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();
    //controlBar.setBounds(bounds.removeFromTop(25));
    //bandControls.setBounds(bounds.removeFromBottom(135));
    //analyzer.setBounds(bounds.removeFromTop(250));
    //globalControls.setBounds(bounds);

    controlBar.setBounds(bounds.removeFromTop(25));
    globalControls.setBounds(bounds.removeFromBottom(140));
    analyzer.setBounds(bounds.removeFromTop(230));
    bandControls.setBounds(bounds);
}
