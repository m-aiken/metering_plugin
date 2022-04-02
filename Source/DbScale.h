/*
  ==============================================================================

    DbScale.h
    Created: 2 Apr 2022 1:03:29am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Tick.h"

//==============================================================================
struct DbScale : juce::Component
{
    void paint(juce::Graphics& g) override;
    int yOffset = 0;
    std::vector<Tick> ticks;
};
