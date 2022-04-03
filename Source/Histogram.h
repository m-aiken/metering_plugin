/*
  ==============================================================================

    Histogram.h
    Created: 2 Apr 2022 12:44:53am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "HistogramEnums.h"
#include "CircularBuffer.h"

//==============================================================================
struct Histogram : juce::Component
{
    Histogram(const juce::String& _label) : label(_label) { }
    void paint(juce::Graphics& g) override;
    void update(const float& inputL, const float& inputR);
    
    void setThreshold(const float& threshAsDecibels);
    void setView(const HistView& v);
    juce::Value& getThresholdValueObject() { return threshold; }
    
private:
    CircularBuffer<float> circularBuffer{776, -48.f};
    
    juce::String label;
    juce::Value threshold;
    
    HistView view;
};
