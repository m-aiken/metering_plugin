/*
  ==============================================================================

    ValueHolder.cpp
    Created: 2 Apr 2022 1:00:45am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "ValueHolder.h"

//==============================================================================
void ValueHolder::setThreshold(const float& threshAsDecibels)
{
    threshold = threshAsDecibels;
}

void ValueHolder::updateHeldValue(const float& input)
{
    currentValue = input;
    
    if (isOverThreshold())
    {
        peakTime = juce::Time::currentTimeMillis();
        if (input > heldValue)
            heldValue = input;
    }
}

bool ValueHolder::isOverThreshold() const
{
    return currentValue > threshold;
}

void ValueHolder::handleOverHoldTime()
{
    heldValue = NegativeInfinity;
}
