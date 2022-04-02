/*
  ==============================================================================

    ValueHolder.h
    Created: 2 Apr 2022 1:00:45am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include "ValueHolderBase.h"

//==============================================================================
struct ValueHolder : ValueHolderBase
{
    ValueHolder() { }
    ~ValueHolder() { }
    
    void setThreshold(const float& threshAsDecibels);
    void updateHeldValue(const float& input);
    bool isOverThreshold() const;
    void handleOverHoldTime() override;
    
private:
    float threshold = 0.f;
};
