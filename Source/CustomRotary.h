/*
  ==============================================================================

    CustomRotary.h
    Created: 2 Apr 2022 1:17:10am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

//==============================================================================
struct CustomRotary : juce::Slider
{
    CustomRotary();
    ~CustomRotary() { setLookAndFeel(nullptr); }
    void paint(juce::Graphics& g) override;
private:
    CustomLookAndFeel lnf;
};
