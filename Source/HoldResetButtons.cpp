/*
  ==============================================================================

    HoldResetButtons.cpp
    Created: 2 Apr 2022 1:42:16am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "HoldResetButtons.h"

//==============================================================================
HoldResetButtons::HoldResetButtons()
{
    addAndMakeVisible(holdButton);
    addAndMakeVisible(resetButton);
}

void HoldResetButtons::resized()
{
    juce::Grid grid;
     
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    
    grid.templateColumns = { Track(Fr(2)), Track(Fr(1)) };
    grid.autoRows = Track(Fr(1));
    grid.items = { juce::GridItem(holdButton), juce::GridItem(resetButton) };
    
    grid.setGap(juce::Grid::Px{4});
    grid.performLayout(getLocalBounds());
}
