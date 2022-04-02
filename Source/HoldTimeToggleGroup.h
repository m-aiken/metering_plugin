/*
  ==============================================================================

    HoldTimeToggleGroup.h
    Created: 2 Apr 2022 1:36:43am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ToggleGroupBase.h"

//==============================================================================
struct HoldTimeToggleGroup : ToggleGroupBase, juce::Component
{
    HoldTimeToggleGroup();
    void resized() override;
    void setSelectedToggleFromState();
    
    CustomToggle optionA{"0s"}, optionB{"0.5s"}, optionC{"2s"}, optionD{"4s"}, optionE{"6s"}, optionF{"inf"};
    std::vector<CustomToggle*> toggles = { &optionA, &optionB, &optionC, &optionD, &optionE, &optionF };
};
