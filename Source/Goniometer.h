/*
  ==============================================================================

    Goniometer.h
    Created: 2 Apr 2022 12:33:09am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
struct Goniometer : juce::Component
{
    void paint(juce::Graphics& g) override;
    void resized() override;
    void update(juce::AudioBuffer<float>& incomingBuffer);
    void setScale(const double& rotaryValue);

private:
    juce::Image canvas;
    juce::AudioBuffer<float> buffer;
    
    double scale;
};
