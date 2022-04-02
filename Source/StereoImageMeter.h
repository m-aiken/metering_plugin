/*
  ==============================================================================

    StereoImageMeter.h
    Created: 2 Apr 2022 12:56:16am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Goniometer.h"
#include "CorrelationMeter.h"

//==============================================================================
struct StereoImageMeter : juce::Component
{
    StereoImageMeter(double _sampleRate, size_t _blockSize);
    void paint(juce::Graphics& g) override;
    void update(juce::AudioBuffer<float>& incomingBuffer);
    void setGoniometerScale(const double& rotaryValue);
private:
    Goniometer goniometer;
    CorrelationMeter correlationMeter;
};
