/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#define MaxDecibels 12.f
#define NegativeInfinity -48.f

//==============================================================================
struct ValueHolderBase : juce::Timer
{
    ValueHolderBase() { startTimerHz(30); }
    ~ValueHolderBase() { stopTimer(); }
    
    void setHoldTime(const long long& ms) { holdTime = ms; }
    float getCurrentValue() const { return currentValue; }
    float getHeldValue() const { return heldValue; }
    
    void timerCallback() override;
    virtual void handleOverHoldTime() = 0;
    
    friend struct DecayingValueHolder;
    friend struct ValueHolder;
private:
    float currentValue = NegativeInfinity;
    float heldValue = NegativeInfinity;
   
    long long now = juce::Time::currentTimeMillis();
    long long peakTime = 0;
    long long holdTime = 500;
};

//==============================================================================
struct DecayingValueHolder : ValueHolderBase
{
    DecayingValueHolder() { setDecayRate(initDecayRate); }
    ~DecayingValueHolder() { }
    
    void updateHeldValue(const float& input);
    void setDecayRate(const float& dbPerSecond);
    
    void handleOverHoldTime() override;
    
private:
    int timerFrequency = 30;
    float initDecayRate = 20.f;
    float decayRatePerFrame;
};

//==============================================================================
struct ValueHolder : ValueHolderBase
{
    ValueHolder() { }
    ~ValueHolder() { }
    
    void setThreshold(const float& threshold) { mThreshold = threshold; }
    void updateHeldValue(const float& input);
    bool getIsOverThreshold() const { return isOverThreshold; }
    
    void handleOverHoldTime() override;
    
private:
    float mThreshold = -9.f;
    bool isOverThreshold = false;
};

//==============================================================================
struct TextMeter : juce::Component
{
    void paint(juce::Graphics& g) override;
    void update(const float& input);
    
private:
    ValueHolder valueHolder;
    DecayingValueHolder decayingValueHolder;
};

//==============================================================================
struct Tick
{
    int y = 0;
    float db = 0.f;
};

struct DbScale : juce::Component
{
    void paint(juce::Graphics& g) override;
    int yOffset = 0;
    std::vector<Tick> ticks;
};

struct Meter : juce::Component
{
    void paint(juce::Graphics& g) override;
    void resized() override;
    void update(const float& newLevel);
    
    std::vector<Tick> ticks;
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
    
    TextMeter textMeter;
    Meter monoMeter;
    DbScale dbScale;
    
#if defined(GAIN_TEST_ACTIVE)
    juce::Slider gainSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment gainAttachment{audioProcessor.apvts, "Gain", gainSlider};
#endif
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PFMProject10AudioProcessorEditor)
};
