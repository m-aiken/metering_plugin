/*
  ==============================================================================

    HistViewToggleGroup.cpp
    Created: 2 Apr 2022 1:36:56am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "HistViewToggleGroup.h"

//==============================================================================
HistViewToggleGroup::HistViewToggleGroup()
{
    for ( auto& toggle : toggles )
    {
        addAndMakeVisible(toggle);
        toggle->setRadioGroupId(5);
    }
}

void HistViewToggleGroup::resized()
{
    juce::Grid grid = generateGrid(toggles);
    grid.performLayout(getLocalBounds());
}

void HistViewToggleGroup::setSelectedToggleFromState()
{
    using nt = juce::NotificationType;
    switch (static_cast<int>(getValueObject().getValue()))
    {
        case 1:  optionA.setToggleState(true, nt::dontSendNotification); break;
        case 2:  optionB.setToggleState(true, nt::dontSendNotification); break;
        default: optionA.setToggleState(true, nt::dontSendNotification); break;
    }
}

juce::Grid HistViewToggleGroup::generateGrid(std::vector<CustomToggle*>& toggles)
{
    juce::Grid grid;
     
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    
    grid.templateColumns = { Track(Fr(1)), Track(Fr(1)) };
    grid.autoRows = Track(Fr(1));
    
    for ( auto& toggle : toggles )
        grid.items.add(juce::GridItem(*toggle));
    
    grid.setGap(juce::Grid::Px{4});
    return grid;
}
