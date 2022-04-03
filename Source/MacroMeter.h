/*
  ==============================================================================

    MacroMeter.h
    Created: 2 Apr 2022 1:07:02am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Channel.h"
#include "TextMeter.h"
#include "Meter.h"
#include "Averager.h"
#include "Tick.h"

//==============================================================================
struct MacroMeter : juce::Component
{
    MacroMeter(const Channel& channel);

    void resized() override;
    void update(const float& input);
    
    std::vector<Tick> getTicks() { return instantMeter.ticks; }
    int getTickYoffset() { return textMeter.getHeight(); }
    
    void setThreshold(const float& threshAsDecibels);
    void setDecayRate(const float& dbPerSecond);
    void setHoldTime(const long long& ms);
    void resetValueHolder();
    void setMeterView(const int& newViewId);
    void setTickVisibility(const bool& toggleState);
    void resizeAverager(const int& durationId);
    
private:
    TextMeter textMeter;
    Meter averageMeter;
    Meter instantMeter;
    
    Averager<float> averager{20, -48.f};
    
    Channel channel;
};
