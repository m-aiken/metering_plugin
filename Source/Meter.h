/*
  ==============================================================================

    Meter.h
    Created: 2 Apr 2022 1:05:20am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Tick.h"
#include "DecayingValueHolder.h"

//==============================================================================
struct Meter : juce::Component
{
    void paint(juce::Graphics& g) override;
    void resized() override;
    void update(const float& newLevel);
    
    void setThreshold(const float& threshAsDecibels);
    void setDecayRate(const float& dbPerSecond);
    void setHoldTime(const long long& ms);
    void resetValueHolder();
    
    void setTickVisibility(const bool& toggleState);
    
    std::vector<Tick> ticks;
private:
    float level = 0.f;
    
    DecayingValueHolder fallingTick;
    bool fallingTickEnabled;
    
    float threshold = 0.f;
};
