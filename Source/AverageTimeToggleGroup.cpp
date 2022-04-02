/*
  ==============================================================================

    AverageTimeToggleGroup.cpp
    Created: 2 Apr 2022 1:36:05am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "AverageTimeToggleGroup.h"

//==============================================================================
AverageTimeToggleGroup::AverageTimeToggleGroup()
{
    for ( auto& toggle : toggles )
    {
        addAndMakeVisible(toggle);
        toggle->setRadioGroupId(2);
    }
}

void AverageTimeToggleGroup::resized()
{
    juce::Grid grid = generateGrid(toggles);
    grid.performLayout(getLocalBounds());
}

void AverageTimeToggleGroup::setSelectedToggleFromState()
{
    using nt = juce::NotificationType;
    switch (static_cast<int>(getValueObject().getValue()))
    {
        case 1:  optionA.setToggleState(true, nt::dontSendNotification); break;
        case 2:  optionB.setToggleState(true, nt::dontSendNotification); break;
        case 3:  optionC.setToggleState(true, nt::dontSendNotification); break;
        case 4:  optionD.setToggleState(true, nt::dontSendNotification); break;
        case 5:  optionE.setToggleState(true, nt::dontSendNotification); break;
        default: optionC.setToggleState(true, nt::dontSendNotification); break;
    }
}
