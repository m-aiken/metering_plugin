/*
  ==============================================================================

    HistogramContainer.h
    Created: 2 Apr 2022 12:48:00am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Histogram.h"
#include "HistogramEnums.h"

//==============================================================================
struct HistogramContainer : juce::Component
{
    HistogramContainer();
    void resized() override;
    
    void update(const HistogramTypes& histoType, const float& inputL, const float& inputR);
    void setThreshold(const HistogramTypes& histoType, const float& threshAsDecibels);
    
    void setView(const int& selectedId);
    juce::Value& getThresholdValueObject(const HistogramTypes& histoType);
    
private:
    Histogram rmsHistogram{"RMS"};
    Histogram peakHistogram{"PEAK"};
        
    HistView view;
};
