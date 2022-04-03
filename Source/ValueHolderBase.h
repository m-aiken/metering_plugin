/*
  ==============================================================================

    ValueHolderBase.h
    Created: 2 Apr 2022 12:58:04am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Globals.h"

//==============================================================================
struct ValueHolderBase : juce::Timer
{
    ValueHolderBase() { startTimerHz(40); }
    ~ValueHolderBase() { stopTimer(); }
    
    void setHoldTime(const juce::int64& ms) { holdTime = ms; }
    juce::int64 getHoldTime() { return holdTime; }
    float getCurrentValue() const { return currentValue; }
    float getHeldValue() const { return heldValue; }
    void reset() { currentValue = Globals::negInf(); }
    
    void timerCallback() override
    {
        if ( getNow() - peakTime > holdTime )
            handleOverHoldTime();
    }
    
    virtual void handleOverHoldTime() = 0;
    
    friend struct DecayingValueHolder;
    friend struct ValueHolder;
private:
    float currentValue = Globals::negInf();
    float heldValue = Globals::negInf();
   
    juce::int64 peakTime = getNow();
    juce::int64 holdTime = 2000;
    
    static juce::int64 getNow() { return juce::Time::currentTimeMillis(); }
};
