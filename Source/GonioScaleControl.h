/*
  ==============================================================================

    GonioScaleControl.h
    Created: 2 Apr 2022 1:46:11am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomRotary.h"
#include "CustomLabel.h"

//==============================================================================
struct GonioScaleControl : juce::Component
{
    GonioScaleControl();
    void resized() override;
    
    CustomRotary gonioScaleKnob;
    
private:
    CustomLabel gonioScaleLabel { "Scale" };
};
