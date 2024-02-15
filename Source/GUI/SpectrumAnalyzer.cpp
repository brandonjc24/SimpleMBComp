/*
  ==============================================================================

    SpectrumAnalyzer.cpp
    Created: 14 Feb 2024 12:22:44pm
    Author:  brand

  ==============================================================================
*/

#include "SpectrumAnalyzer.h"
#include <JuceHeader.h>
#include "../GUI/Utilities.h"
#include "../DSP/Params.h"

SpectrumAnalyzer::SpectrumAnalyzer(SimpleMBCompAudioProcessor& p) :
    audioProcessor(p),
    leftPathProducer(audioProcessor.leftChannelFifo),
    rightPathProducer(audioProcessor.rightChannelFifo)
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->addListener(this);
    }
    using namespace Params;
    const auto& paramNames = GetParams();

    auto floatHelper = [&apvts = audioProcessor.apvts , &paramNames](auto& param, const auto& paramName)
        { param = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(paramNames.at(paramName))); };

    floatHelper(lowMidXoverParam, Names::Low_Mid_Crossover_Freq);
    floatHelper(midHighXoverParam, Names::Mid_High_Crossover_Freq);

    floatHelper(lowThresholdParam, Names::Threshold_Low_Band);
    floatHelper(midThresholdParam, Names::Threshold_Mid_Band);
    floatHelper(highThresholdParam, Names::Threshold_High_Band);


    startTimerHz(60);
}

SpectrumAnalyzer::~SpectrumAnalyzer()
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params)
    {
        param->removeListener(this);
    }
}

void SpectrumAnalyzer::drawFFTAnalysis(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    using namespace juce;
    auto responseArea = getAnalysisArea(bounds);

    Graphics::ScopedSaveState sss(g);
    g.reduceClipRegion(responseArea);

    auto leftChannelFFTPath = leftPathProducer.getPath();
    leftChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), 0)); // responseArea.getY()));

    g.setColour(juce::Colours::forestgreen); // g.setColour(Colour(97u, 18u, 167u))-
    g.strokePath(leftChannelFFTPath, PathStrokeType(1.f));

    auto rightChannelFFTPath = rightPathProducer.getPath();
    rightChannelFFTPath.applyTransform(AffineTransform().translation(responseArea.getX(), 0)); // responseArea.getY()));

    g.setColour(juce::Colours::forestgreen); //g.setColour(Colour(215u, 201u, 134u));
    g.strokePath(rightChannelFFTPath, PathStrokeType(1.f));
}

void SpectrumAnalyzer::paint(juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::pink); //lightgrey

    auto bounds = drawModuleBackground(g, getLocalBounds());

    drawBackgroundGrid(g, bounds);

    if (shouldShowFFTAnalysis)
    {
        drawFFTAnalysis(g, bounds);
    }

    drawCrossovers(g, bounds);
    drawTextLabels(g, bounds);
}

void SpectrumAnalyzer::drawCrossovers(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    using namespace juce;
    bounds = getAnalysisArea(bounds);

    const auto top = bounds.getY();
    const auto bottom = bounds.getBottom();
    const auto left = bounds.getX();
    const auto right = bounds.getRight();

    auto mapX = [left = bounds.getX(), width = bounds.getWidth()](float frequency)
        {
            auto normX = juce::mapFromLog10(frequency, MIN_FREQUENCY, MAX_FREQUENCY);
            return left + width * normX;
        };


    auto xColour = juce::Colours::darkslategrey;
    auto lowMidX = mapX(lowMidXoverParam->get());
    g.setColour(xColour);
    //g.drawVerticalLine(lowMidX, top, bottom);
    g.fillRect(int(lowMidX), int(top), 2, int(bottom - top));
    auto midHighX = mapX(midHighXoverParam->get());
    //g.drawVerticalLine(midHighX, top, bottom);
    g.fillRect(int(midHighX), int(top), 2, int(bottom - top));

    auto mapY = [bottom, top](float db)
        {
            return jmap(db, NEGATIVE_INFINITY, MAX_DECIBELS, (float)bottom, (float)top);
        };

    auto zeroDb = mapY(0.f);
    g.setColour(juce::Colours::red.withAlpha(0.3f));
    g.fillRect(Rectangle<float>::leftTopRightBottom(left, zeroDb, lowMidX, mapY(lowBandGR)));
    g.fillRect(Rectangle<float>::leftTopRightBottom(lowMidX, zeroDb, midHighX, mapY(midBandGR)));
    g.fillRect(Rectangle<float>::leftTopRightBottom(midHighX, zeroDb, right, mapY(highBandGR)));


    g.setColour(xColour);
    //g.drawHorizontalLine(mapY(lowThresholdParam->get()), left, lowMidX);
    g.fillRect(int(left), int(mapY(lowThresholdParam->get())), int(lowMidX-left)+1,2);
    //g.drawHorizontalLine(mapY(midThresholdParam->get()), lowMidX, midHighX);
    g.fillRect(int(lowMidX), int(mapY(midThresholdParam->get())), int(midHighX - lowMidX)+1, 2);
    //g.drawHorizontalLine(mapY(highThresholdParam->get()), midHighX, right);
    g.fillRect(int(midHighX), int(mapY(highThresholdParam->get())), int(right - midHighX)+1, 2);
}

void SpectrumAnalyzer::update(const std::vector<float>& values)
{
    enum
    {
        LowBandIn,
        LowBandOut,
        MidBandIn,
        MidBandOut,
        HighBandIn,
        HighBandOut
    };
    lowBandGR = values[LowBandOut] - values[LowBandIn];
    midBandGR = values[MidBandOut] - values[MidBandIn];
    highBandGR = values[HighBandOut] - values[HighBandIn];

    repaint();
}

std::vector<float> SpectrumAnalyzer::getFrequencies()
{
    return std::vector<float>
    {
        20, /*30, 40,*/ 50, 100,
            200, /*300, 400,*/ 500, 1000,
            2000, /*3000, 4000,*/ 5000, 10000,
            20000
    };
}

std::vector<float> SpectrumAnalyzer::getGains()
{
    std::vector<float>  values;
    auto increment = MAX_DECIBELS;
    for (auto db = NEGATIVE_INFINITY; db <= MAX_DECIBELS; db += increment)
        values.push_back(db);
    return values;
}

std::vector<float> SpectrumAnalyzer::getXs(const std::vector<float>& freqs, float left, float width)
{
    std::vector<float> xs;
    for (auto f : freqs)
    {
        auto normX = juce::mapFromLog10(f, MIN_FREQUENCY, MAX_FREQUENCY);
        xs.push_back(left + width * normX);
    }

    return xs;
}

void SpectrumAnalyzer::drawBackgroundGrid(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    using namespace juce;
    auto freqs = getFrequencies();
    auto gridColour = juce::Colours::ghostwhite;
    
    auto renderArea = getAnalysisArea(bounds);
    auto left = renderArea.getX();
    auto right = renderArea.getRight();
    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();
    g.setColour(juce::Colours::lightgrey);
    g.fillRect(left, top, right - left, bottom - top);
    auto xs = getXs(freqs, left, width);

    g.setColour(gridColour);
    for (auto x : xs)
    {
        g.drawVerticalLine(x, top, bottom);
    }

    auto gain = getGains();

    for (auto gDb : gain)
    {
        auto y = jmap(gDb, NEGATIVE_INFINITY, MAX_DECIBELS, (float)bottom, (float)top);
        g.setColour(gDb == 0.f ? juce::Colours::slateblue : gridColour);
        g.drawHorizontalLine(y, left, right);
    }
}

void SpectrumAnalyzer::drawTextLabels(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    auto textColor = juce::Colours::white;
    using namespace juce;
    g.setColour(textColor);
    const int fontHeight = 11;
    g.setFont(fontHeight);

    auto renderArea = getAnalysisArea(bounds);
    auto left = renderArea.getX();

    auto top = renderArea.getY();
    auto bottom = renderArea.getBottom();
    auto width = renderArea.getWidth();

    auto freqs = getFrequencies();
    auto xs = getXs(freqs, left, width);

    for (int i = 0; i < freqs.size(); ++i)
    {
        auto f = freqs[i];
        auto x = xs[i];

        bool addK = false;
        String str;
        if (f > 999.f)
        {
            addK = true;
            f /= 1000.f;
        }

        str << f;
        if (addK)
            str << "k";
        str << "Hz";

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<int> r;

        r.setSize(textWidth, fontHeight);
        r.setCentre(x, 0);
        r.setY(bounds.getY());

        g.drawFittedText(str, r, juce::Justification::centred, 1);
    }

    auto gain = getGains();

    for (auto gDb : gain)
    {
        //auto y = jmap(gDb, -24.f, 24.f, float(bottom), float(top));
        auto y = jmap(gDb, NEGATIVE_INFINITY, MAX_DECIBELS, (float)bottom, (float)top);

        String str;
        if (gDb > 0)
            str << "+";
        str << gDb;

        auto textWidth = g.getCurrentFont().getStringWidth(str);

        Rectangle<int> r;
        r.setSize(textWidth, fontHeight);
        r.setX(bounds.getRight() - textWidth);
        r.setCentre(r.getCentreX(), y);
        g.drawFittedText(str, r, juce::Justification::centredLeft, 1);

        r.setX(bounds.getX() + 1);
        g.drawFittedText(str, r, juce::Justification::centredLeft, 1);
    }
}

void SpectrumAnalyzer::resized()
{
    using namespace juce;
    auto bounds = getLocalBounds();
    auto fftBounds = getAnalysisArea(bounds).toFloat();
    auto negInf = jmap(bounds.toFloat().getBottom(),
                        fftBounds.getBottom(), fftBounds.getY(),
                        NEGATIVE_INFINITY, 
                        MAX_DECIBELS);

    DBG("Neg: " << negInf);
    leftPathProducer.updateNEgativeInfinity(negInf);
    rightPathProducer.updateNEgativeInfinity(negInf);

}

void SpectrumAnalyzer::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}



void SpectrumAnalyzer::timerCallback()
{
    if (shouldShowFFTAnalysis)
    {
        auto bounds = getLocalBounds();
        auto fftBounds = getAnalysisArea(bounds).toFloat();
        fftBounds.setBottom(bounds.getBottom());
        auto sampleRate = audioProcessor.getSampleRate();

        leftPathProducer.process(fftBounds, sampleRate);
        rightPathProducer.process(fftBounds, sampleRate);
    }

    if (parametersChanged.compareAndSetBool(false, true))
    {

    }

    repaint();
}

juce::Rectangle<int> SpectrumAnalyzer::getRenderArea(juce::Rectangle<int> bounds)
{
    bounds.removeFromTop(12);
    bounds.removeFromBottom(2);
    bounds.removeFromLeft(20);
    bounds.removeFromRight(20);

    return bounds;
}

juce::Rectangle<int> SpectrumAnalyzer::getAnalysisArea(juce::Rectangle<int> bounds)
{
    bounds = getRenderArea(bounds);
    bounds.removeFromTop(4);
    bounds.removeFromBottom(4);
    return bounds;
}
