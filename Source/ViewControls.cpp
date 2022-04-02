/*
  ==============================================================================

    ViewControls.cpp
    Created: 2 Apr 2022 1:46:21am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "ViewControls.h"

//==============================================================================
ViewControls::ViewControls()
{
    addAndMakeVisible(meterViewLabel);
    addAndMakeVisible(meterView);
    addAndMakeVisible(histViewLabel);
    addAndMakeVisible(histView);
    
    addAndMakeVisible(lineBreak);
}

void ViewControls::resized()
{
    auto bounds = getLocalBounds();
    auto buttonHeight = bounds.getHeight() / 4.5f;
    
    juce::Grid grid;
     
    using Track = juce::Grid::TrackInfo;
    using Px = juce::Grid::Px;
    
    grid.autoColumns = Track(Px(bounds.getWidth()));
    
    grid.templateRows =
    {
        Track(Px(buttonHeight)),
        Track(Px(buttonHeight)),
        Track(Px(buttonHeight / 2)), // line break
        Track(Px(buttonHeight)),
        Track(Px(buttonHeight))
    };
    
    grid.items =
    {
        juce::GridItem(meterViewLabel),
        juce::GridItem(meterView),
        juce::GridItem(lineBreak),
        juce::GridItem(histViewLabel),
        juce::GridItem(histView)
    };
    
    grid.performLayout(bounds);
}
