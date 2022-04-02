/*
  ==============================================================================

    ThresholdSlider.h
    Created: 2 Apr 2022 1:10:42am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

#define MaxDecibels 6.f
#define NegativeInfinity -48.f

//==============================================================================
struct ThresholdSlider : juce::Slider
{
    ThresholdSlider();
    ~ThresholdSlider();
    
    void paint(juce::Graphics& g) override;
    
private:
    CustomLookAndFeel lnf;
};
