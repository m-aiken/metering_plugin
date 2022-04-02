/*
  ==============================================================================

    StereoMeter.h
    Created: 2 Apr 2022 1:11:51am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ThresholdSlider.h"
#include "MacroMeter.h"
#include "DbScale.h"
#include "CustomLookAndFeel.h"

//==============================================================================
struct StereoMeter : juce::Component
{
    StereoMeter(const juce::String& labelText);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void update(const float& inputL, const float& inputR);
    
    void setThreshold(const float& threshAsDecibels);
    void setDecayRate(const int& selectedId);
    
    void setTickVisibility(const bool& toggleState);
    void setTickHoldTime(const int& selectedId);
    void resetValueHolder();
    void setMeterView(const int& newViewId);
    
    void resizeAverager(const int& durationId);
    
    ThresholdSlider threshCtrl;
private:
    MacroMeter macroMeterL{ Channel::Left };
    DbScale dbScale;
    MacroMeter macroMeterR{ Channel::Right };
    
    juce::String label;
    
    float dbScaleLabelCrossover = 0.94f;
    
    CustomLookAndFeel customStyle;
};
