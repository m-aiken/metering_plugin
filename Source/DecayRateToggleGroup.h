/*
  ==============================================================================

    DecayRateToggleGroup.h
    Created: 2 Apr 2022 1:35:51am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ToggleGroupBase.h"

//==============================================================================
struct DecayRateToggleGroup : ToggleGroupBase, juce::Component
{
    DecayRateToggleGroup();
    void resized() override;
    void setSelectedToggleFromState();
    
    CustomToggle optionA{"-3"}, optionB{"-6"}, optionC{"-12"}, optionD{"-24"}, optionE{"-36"};
    std::vector<CustomToggle*> toggles = { &optionA, &optionB, &optionC, &optionD, &optionE };
};
