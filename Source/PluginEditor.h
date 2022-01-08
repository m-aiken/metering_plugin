/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
struct Meter : juce::Component
{
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        auto h = bounds.getHeight();
        
        g.setColour(juce::Colours::lightblue);
        // gainToDecibels default range: 0.f to -100.f (default negativeInf)
        auto jmap = juce::jmap<float>(level, -100.f, 0.f, h, 0);
        g.fillRect(bounds.withHeight(h * jmap).withY(jmap));
    }
    
    void update(float& newLevel)
    {
        level = newLevel;
        repaint();
    }
    
private:
    float level{0};
};
//==============================================================================
/**
*/
class PFMProject10AudioProcessorEditor  : public juce::AudioProcessorEditor, juce::Timer
{
public:
    PFMProject10AudioProcessorEditor (PFMProject10AudioProcessor&);
    ~PFMProject10AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void timerCallback() override;
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PFMProject10AudioProcessor& audioProcessor;
    
    juce::AudioBuffer<float> incomingBuffer;
    
    Meter monoMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PFMProject10AudioProcessorEditor)
};
