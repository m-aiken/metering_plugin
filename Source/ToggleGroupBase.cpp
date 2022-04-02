/*
  ==============================================================================

    ToggleGroupBase.cpp
    Created: 2 Apr 2022 1:22:34am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "ToggleGroupBase.h"

//==============================================================================
juce::Grid ToggleGroupBase::generateGrid(std::vector<CustomToggle*>& toggles)
{
    juce::Grid grid;
     
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    
    grid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
    grid.autoRows = Track(Fr(1));
    
    for ( auto& toggle : toggles )
        grid.items.add(juce::GridItem(*toggle));
    
    grid.setGap(juce::Grid::Px{4});
    return grid;
}
