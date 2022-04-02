/*
  ==============================================================================

    MeterViewToggleGroup.cpp
    Created: 2 Apr 2022 1:36:24am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "MeterViewToggleGroup.h"

//==============================================================================
MeterViewToggleGroup::MeterViewToggleGroup()
{
    for ( auto& toggle : toggles )
    {
        addAndMakeVisible(toggle);
        toggle->setRadioGroupId(3);
    }
}

void MeterViewToggleGroup::resized()
{
    juce::Grid grid = generateGrid(toggles);
    grid.performLayout(getLocalBounds());
}

void MeterViewToggleGroup::setSelectedToggleFromState()
{
    using nt = juce::NotificationType;
    switch (static_cast<int>(getValueObject().getValue()))
    {
        case 1:  optionA.setToggleState(true, nt::dontSendNotification); break;
        case 2:  optionB.setToggleState(true, nt::dontSendNotification); break;
        case 3:  optionC.setToggleState(true, nt::dontSendNotification); break;
        default: optionA.setToggleState(true, nt::dontSendNotification); break;
    }
}
