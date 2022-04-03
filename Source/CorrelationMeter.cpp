/*
  ==============================================================================

    CorrelationMeter.cpp
    Created: 2 Apr 2022 12:54:22am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "CorrelationMeter.h"
#include "MyColours.h"
#include "Globals.h"

//==============================================================================
CorrelationMeter::CorrelationMeter(double _sampleRate, size_t _blockSize)
    : sampleRate(_sampleRate), blockSize(_blockSize)
{
    prepareFilters();
}

void CorrelationMeter::prepareFilters()
{
    juce::dsp::ProcessSpec spec;
    spec.numChannels = 1;
    
    using FilterDesign = juce::dsp::FilterDesign<float>;
    using WindowingFunction = juce::dsp::WindowingFunction<float>;
    
    auto coefficientsPtr = FilterDesign::designFIRLowpassWindowMethod(100.f,                                     // frequency
                                                                      sampleRate,                                // sample rate
                                                                      2,                                         // order
                                                                      WindowingFunction::WindowingMethod::hann); // windowing method
    
    for ( auto& filter : filters )
    {
        filter.prepare(spec);
        filter.coefficients = coefficientsPtr;
    }
}

void CorrelationMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto width = bounds.getWidth();
    auto height = bounds.getHeight();
    auto padding = width / 10;
    auto meterWidth = width - (padding * 2);
    
    // labels
    g.setColour(MyColours::getColour(MyColours::Text));
    g.setFont(Globals::font());
    // draw fitted text args = text, x, y, width, height, justification, maxNumLines
    g.drawFittedText("-1", 0, 0, padding, height, juce::Justification::centred, 1);
    g.drawFittedText("+1", width - padding, 0, padding, height, juce::Justification::centred, 1);
    
    // meter background
    auto meterBounds = juce::Rectangle<int>(bounds.getCentreX() - (meterWidth / 2), // x
                                            bounds.getY(),                          // y
                                            meterWidth,                             // width
                                            height);                                // height

    auto shadow = MyColours::getDropShadow();
    shadow.drawForRectangle(g, meterBounds);
    
    // meters
    auto gradient = MyColours::getMeterGradient(bounds.getCentreX(), bounds.getRight(), MyColours::GradientOrientation::Horizontal);
    g.setGradientFill(gradient);
    
    juce::Rectangle<int> averageCorrelationMeter = paintMeter(meterBounds,            // container bounds
                                                   meterBounds.getY(),                // y
                                                   static_cast<int>(height * 0.2f),   // height
                                                   averagedCorrelation.getAverage()); // value
    
    juce::Rectangle<int> instantCorrelationMeter = paintMeter(meterBounds,                                           // container bounds
                                                              averageCorrelationMeter.getBottom() + (height * 0.1f), // y
                                                              static_cast<int>(height * 0.7f),                       // height
                                                              instantaneousCorrelation.getAverage());                // value
    
    g.fillRect(averageCorrelationMeter);
    g.fillRect(instantCorrelationMeter);
}

juce::Rectangle<int> CorrelationMeter::paintMeter(const juce::Rectangle<int>& containerBounds, const int& y, const int& height, const float& value)
{
    auto jmap = juce::jmap<float>(value,                       // source
                                  -1.f,                        // source min
                                  1.f,                         // source max
                                  containerBounds.getX(),      // target min
                                  containerBounds.getRight()); // target max
    
    juce::Rectangle<int> rectangle;
    rectangle.setY(y);
    rectangle.setHeight(height);
    
    if ( jmap > containerBounds.getCentreX() )
    {
        rectangle.setX(containerBounds.getCentreX());
        rectangle.setWidth(std::floor(jmap) - containerBounds.getCentreX());
    }
    else
    {
        rectangle.setX(std::floor(jmap));
        rectangle.setWidth(containerBounds.getCentreX() - std::floor(jmap));
    }
    
    return rectangle;
}

void CorrelationMeter::update(juce::AudioBuffer<float>& incomingBuffer)
{
    for ( auto i = 0; i < incomingBuffer.getNumSamples(); ++i )
    {
        auto sampleL = incomingBuffer.getSample(0, i);
        auto sampleR = incomingBuffer.getSample(1, i);
        
        auto denominator = std::sqrt( filters[1].processSample( std::pow(sampleL, 2) ) * filters[2].processSample( std::pow(sampleR, 2) ) );
        
        if ( denominator != 0.f && !std::isinf(denominator) )
        {
            auto numerator = filters[0].processSample(sampleL * sampleR);
            auto correlation = numerator / denominator;
            
            instantaneousCorrelation.add(correlation);
            averagedCorrelation.add(correlation);
        }
        else
        {
            instantaneousCorrelation.add(0.f);
            averagedCorrelation.add(0.f);
        }
    }
    
    repaint();
}
