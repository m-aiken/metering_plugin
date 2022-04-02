/*
  ==============================================================================

    DecayingValueHolder.cpp
    Created: 2 Apr 2022 12:59:39am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "DecayingValueHolder.h"

#define MaxDecibels 6.f
#define NegativeInfinity -48.f

//==============================================================================
void DecayingValueHolder::updateHeldValue(const float& input)
{
    if (input > currentValue)
    {
        peakTime = getNow();
        currentValue = input;
        resetDecayRateMultiplier();
    }
}

void DecayingValueHolder::setDecayRate(const float& dbPerSecond)
{
    decayRatePerFrame = dbPerSecond / timerFrequency;
}

void DecayingValueHolder::handleOverHoldTime()
{
    currentValue = juce::jlimit(NegativeInfinity,
                                MaxDecibels,
                                currentValue - (decayRatePerFrame * decayRateMultiplier));
    
    decayRateMultiplier *= 1.04f;
    
    if ( currentValue == NegativeInfinity )
        resetDecayRateMultiplier();
}
