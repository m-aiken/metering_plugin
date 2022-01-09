/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#define MaxDb 12.f
#define NegativeInf -66.f

//==============================================================================
struct Meter : juce::Component
{
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds();
        auto h = bounds.getHeight();
        
        g.setColour(juce::Colours::lightgrey);
        g.fillRect(bounds);
        
        g.setColour(juce::Colours::green);
        // jmap args ( sourceValue, sourceRangeMin, sourceRangeMax, targetRangeMin, targetRangeMax)
        auto jmap = juce::jmap<float>(level, NegativeInf, MaxDb, h, 0);
        g.fillRect(bounds.withHeight(h * jmap).withY(jmap));
    }
    
    void update(float& newLevel)
    {
        level = newLevel;
        repaint();
    }
    
private:
    float level = 0.f;
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
