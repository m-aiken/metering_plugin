/*
  ==============================================================================

    AverageTimeToggleGroup.h
    Created: 2 Apr 2022 1:36:05am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ToggleGroupBase.h"

//==============================================================================
struct AverageTimeToggleGroup : ToggleGroupBase, juce::Component
{
    AverageTimeToggleGroup();
    void resized() override;
    void setSelectedToggleFromState();
    
    CustomToggle optionA{"100"}, optionB{"250"}, optionC{"500"}, optionD{"1000"}, optionE{"2000"};
    std::vector<CustomToggle*> toggles = { &optionA, &optionB, &optionC, &optionD, &optionE };
};
