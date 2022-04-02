/*
  ==============================================================================

    HoldResetButtons.h
    Created: 2 Apr 2022 1:42:16am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomToggle.h"
#include "CustomTextBtn.h"

//==============================================================================
struct HoldResetButtons : juce::Component
{
    HoldResetButtons();
    void resized() override;
    
    CustomToggle holdButton { "HOLD" };
    CustomTextBtn resetButton { "RESET" };
};
