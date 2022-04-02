/*
  ==============================================================================

    CorrelationMeter.h
    Created: 2 Apr 2022 12:54:22am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Averager.h"

//==============================================================================
struct CorrelationMeter : juce::Component
{
    CorrelationMeter(double _sampleRate, size_t _blockSize);
    void prepareFilters();
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> paintMeter(const juce::Rectangle<int>& containerBounds, const int& y, const int& height, const float& value);
    void update(juce::AudioBuffer<float>& incomingBuffer);
    
private:
    using FilterType = juce::dsp::FIR::Filter<float>;
    std::array<FilterType, 3> filters;
    
    double sampleRate;
    size_t blockSize;
    
    Averager<float> instantaneousCorrelation{ blockSize, 0.f };
    Averager<float> averagedCorrelation{ blockSize * 6, 0.f };
};
