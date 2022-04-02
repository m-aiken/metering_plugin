/*
  ==============================================================================

    TimeControls.cpp
    Created: 2 Apr 2022 1:42:34am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "TimeControls.h"

//==============================================================================
TimeControls::TimeControls()
{
    addAndMakeVisible(holdTimeLabel);
    addAndMakeVisible(decayRateLabel);
    addAndMakeVisible(avgDurationLabel);
    
    addAndMakeVisible(holdTime);
    addAndMakeVisible(decayRate);
    addAndMakeVisible(avgDuration);
    
    addAndMakeVisible(lineBreak1);
    addAndMakeVisible(lineBreak2);
}

void TimeControls::resized()
{
    auto bounds = getLocalBounds();
    auto buttonHeight = bounds.getHeight() / 10;
    
    juce::Grid grid;
     
    using Track = juce::Grid::TrackInfo;
    using Px = juce::Grid::Px;
    
    grid.autoColumns = Track(Px(bounds.getWidth()));
    grid.templateRows =
    {
        Track(Px(buttonHeight)),
        Track(Px(buttonHeight * 2)),
        Track(Px(buttonHeight / 2)), // line break
        Track(Px(buttonHeight)),
        Track(Px(buttonHeight * 2)),
        Track(Px(buttonHeight / 2)), // line break
        Track(Px(buttonHeight)),
        Track(Px(buttonHeight * 2))
    };
    
    grid.items =
    {
        juce::GridItem(holdTimeLabel),
        juce::GridItem(holdTime),
        juce::GridItem(lineBreak1),
        juce::GridItem(decayRateLabel),
        juce::GridItem(decayRate),
        juce::GridItem(lineBreak2),
        juce::GridItem(avgDurationLabel),
        juce::GridItem(avgDuration)
    };
    
    grid.performLayout(bounds);
}
