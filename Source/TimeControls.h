/*
  ==============================================================================

    TimeControls.h
    Created: 2 Apr 2022 1:42:34am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "HoldTimeToggleGroup.h"
#include "CustomToggle.h"
#include "CustomTextBtn.h"
#include "DecayRateToggleGroup.h"
#include "AverageTimeToggleGroup.h"
#include "CustomLabel.h"
#include "LineBreak.h"

//==============================================================================
struct TimeControls : juce::Component
{
    TimeControls();
    void resized() override;
    
    HoldTimeToggleGroup holdTime;
    CustomToggle holdButton { "HOLD" };
    CustomTextBtn holdResetButton { "RESET" };
    
    DecayRateToggleGroup decayRate;
    AverageTimeToggleGroup avgDuration;
    
private:
    CustomLabel holdTimeLabel { "Hold Time" };
    CustomLabel decayRateLabel { "Decay Rate (dB/s)" };
    CustomLabel avgDurationLabel { "Average Duration (ms)" };
    
    LineBreak lineBreak1, lineBreak2;
};
