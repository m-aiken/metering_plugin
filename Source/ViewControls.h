/*
  ==============================================================================

    ViewControls.h
    Created: 2 Apr 2022 1:46:21am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "MeterViewToggleGroup.h"
#include "HistViewToggleGroup.h"
#include "CustomLabel.h"
#include "LineBreak.h"

//==============================================================================
struct ViewControls : juce::Component
{
    ViewControls();
    void resized() override;
            
    MeterViewToggleGroup meterView;
    HistViewToggleGroup histView;
    
private:
    CustomLabel meterViewLabel { "Meter View" };
    CustomLabel histViewLabel { "Histogram View" };
    
    LineBreak lineBreak;
};
