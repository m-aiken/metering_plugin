/*
  ==============================================================================

    HistViewToggleGroup.h
    Created: 2 Apr 2022 1:36:56am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ToggleGroupBase.h"

//==============================================================================
struct HistViewToggleGroup : ToggleGroupBase, juce::Component
{
    HistViewToggleGroup();
    void resized() override;
    void setSelectedToggleFromState();
    
    juce::Grid generateGrid(std::vector<CustomToggle*>& toggles) override;
    
    CustomToggle optionA{"Rows"}, optionB{"Columns"};
    std::vector<CustomToggle*> toggles = { &optionA, &optionB };
};
