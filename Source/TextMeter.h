/*
  ==============================================================================

    TextMeter.h
    Created: 2 Apr 2022 1:01:42am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ValueHolder.h"

//==============================================================================
struct TextMeter : juce::Component
{
    void paint(juce::Graphics& g) override;
    void update(const float& input);
    void setThreshold(const float& threshAsDecibels);
    
private:
    ValueHolder valueHolder;
};
