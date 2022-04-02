/*
  ==============================================================================

    CustomLabel.h
    Created: 2 Apr 2022 1:16:43am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
struct CustomLabel : juce::Label
{
    CustomLabel(const juce::String& labelText);
    void paint(juce::Graphics& g) override;
};
