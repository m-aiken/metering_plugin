/*
  ==============================================================================

    DecayRateToggleGroup.cpp
    Created: 2 Apr 2022 1:35:51am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "DecayRateToggleGroup.h"

//==============================================================================
DecayRateToggleGroup::DecayRateToggleGroup()
{
    for ( auto& toggle : toggles )
    {
        addAndMakeVisible(toggle);
        toggle->setRadioGroupId(1);
    }
}

void DecayRateToggleGroup::resized()
{
    juce::Grid grid = generateGrid(toggles);
    grid.performLayout(getLocalBounds());
}

void DecayRateToggleGroup::setSelectedToggleFromState()
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
