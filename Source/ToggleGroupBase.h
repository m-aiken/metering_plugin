/*
  ==============================================================================

    ToggleGroupBase.h
    Created: 2 Apr 2022 1:22:34am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomToggle.h"

//==============================================================================
struct ToggleGroupBase
{
    virtual ~ToggleGroupBase() { }
    virtual juce::Grid generateGrid(std::vector<CustomToggle*>& toggles);
    juce::Value& getValueObject() { return selectedValue; }
    void setSelectedValue(const int& selectedId) { selectedValue.setValue(selectedId); }
    
private:
    juce::Value selectedValue;
};
