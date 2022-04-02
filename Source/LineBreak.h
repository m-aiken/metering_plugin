/*
  ==============================================================================

    LineBreak.h
    Created: 2 Apr 2022 1:42:23am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MyColours.h"

//==============================================================================
struct LineBreak : juce::Component
{
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        g.setColour(MyColours::getColour(MyColours::Text).withAlpha(0.025f));
        g.drawLine(0, bounds.getCentreY(), bounds.getRight(), bounds.getCentreY(), 2.f);
    }
};
