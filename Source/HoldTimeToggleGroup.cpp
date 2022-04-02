/*
  ==============================================================================

    HoldTimeToggleGroup.cpp
    Created: 2 Apr 2022 1:36:43am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "HoldTimeToggleGroup.h"

//==============================================================================
HoldTimeToggleGroup::HoldTimeToggleGroup()
{
    for ( auto& toggle : toggles )
    {
        addAndMakeVisible(toggle);
        toggle->setRadioGroupId(4);
    }
}

void HoldTimeToggleGroup::resized()
{
    juce::Grid grid = generateGrid(toggles);
    grid.performLayout(getLocalBounds());
}

void HoldTimeToggleGroup::setSelectedToggleFromState()
{
    using nt = juce::NotificationType;
    switch (static_cast<int>(getValueObject().getValue()))
    {
        case 1:  optionA.setToggleState(true, nt::dontSendNotification); break;
        case 2:  optionB.setToggleState(true, nt::dontSendNotification); break;
        case 3:  optionC.setToggleState(true, nt::dontSendNotification); break;
        case 4:  optionD.setToggleState(true, nt::dontSendNotification); break;
        case 5:  optionE.setToggleState(true, nt::dontSendNotification); break;
        case 6:  optionF.setToggleState(true, nt::dontSendNotification); break;
        default: optionB.setToggleState(true, nt::dontSendNotification); break;
    }
}
