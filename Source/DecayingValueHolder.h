/*
  ==============================================================================

    DecayingValueHolder.h
    Created: 2 Apr 2022 12:59:39am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include "ValueHolderBase.h"

//==============================================================================
struct DecayingValueHolder : ValueHolderBase
{
    DecayingValueHolder() { setDecayRate(initDecayRate); }
    ~DecayingValueHolder() = default;
    
    void updateHeldValue(const float& input);
    void setDecayRate(const float& dbPerSecond);
    void handleOverHoldTime() override;
    
private:
    int timerFrequency = 40;
    float initDecayRate = 12.f;
    float decayRatePerFrame = 0.f;
    float decayRateMultiplier = 1.f;
    
    void resetDecayRateMultiplier() { decayRateMultiplier = 1.f; }
};
