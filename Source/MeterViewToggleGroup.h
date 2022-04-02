/*
  ==============================================================================

    MeterViewToggleGroup.h
    Created: 2 Apr 2022 1:36:24am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ToggleGroupBase.h"

//==============================================================================
struct MeterViewToggleGroup : ToggleGroupBase, juce::Component
{
    MeterViewToggleGroup();
    void resized() override;
    void setSelectedToggleFromState();
    
    CustomToggle optionA{"Both"}, optionB{"Peak"}, optionC{"Avg"};
    std::vector<CustomToggle*> toggles = { &optionA, &optionB, &optionC };
};
