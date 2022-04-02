/*
  ==============================================================================

    CustomToggle.h
    Created: 2 Apr 2022 1:16:50am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

//==============================================================================
struct CustomToggle : juce::ToggleButton
{
    CustomToggle(const juce::String& buttonText);
    ~CustomToggle() { setLookAndFeel(nullptr); }
    void paint(juce::Graphics& g) override;
private:
    CustomLookAndFeel lnf;
};
