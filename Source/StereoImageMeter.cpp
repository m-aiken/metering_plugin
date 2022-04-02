/*
  ==============================================================================

    StereoImageMeter.cpp
    Created: 2 Apr 2022 12:56:16am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "StereoImageMeter.h"

//==============================================================================
StereoImageMeter::StereoImageMeter(double _sampleRate, size_t _blockSize)
    : correlationMeter(_sampleRate, _blockSize)
{
    addAndMakeVisible(goniometer);
    addAndMakeVisible(correlationMeter);
}

void StereoImageMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto goniometerDims = bounds.getWidth();
    
    goniometer.setBounds(0, 0, goniometerDims, goniometerDims);
    correlationMeter.setBounds(0, goniometer.getBottom(), goniometerDims, 20);
}

void StereoImageMeter::update(juce::AudioBuffer<float>& incomingBuffer)
{
    goniometer.update(incomingBuffer);
    correlationMeter.update(incomingBuffer);
}

void StereoImageMeter::setGoniometerScale(const double& rotaryValue)
{
    goniometer.setScale(rotaryValue);
}
