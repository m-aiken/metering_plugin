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
template<typename T>
struct Averager
{
    Averager(size_t numElements, T initialValue)
    {
        resize(numElements, initialValue);
    }

    void clear(T initialValue)
    {
        container.assign(getSize(), initialValue);
        
        auto initSum = std::accumulate(container.begin(), container.end(), 0);
        runningTotal = static_cast<float>(initSum);
        
        computeAverage();
    }
    
    void resize(size_t s, T initialValue)
    {
        container.resize(s, initialValue);
        clear(initialValue);
    }

    void add(T t)
    {
        auto idx = writeIndex.load();
        auto sum = runningTotal.load();
        
        sum -= static_cast<float>(container[idx]);
        container[idx] = t;
        sum += static_cast<float>(container[idx]);
        runningTotal = sum;
        
        writeIndex = (idx + 1) % getSize();
        
        computeAverage();
    }
    
    float getAverage() const { return average.load(); }
    
    size_t getSize() const { return container.size(); }
    
    void computeAverage() { average = runningTotal.load() / getSize(); }
    
private:
    std::vector<T> container;
    std::atomic<int> writeIndex = 0;
    std::atomic<float> runningTotal = 0.f;
    std::atomic<float> average = 0.f;
};

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
    float initDecayRate = 30.f;
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
    
    DecayingValueHolder fallingTick;
};

//==============================================================================
struct MacroMeter : juce::Component
{
    MacroMeter(const int& channel);

    void resized() override;
    void update(const float& input);
    
    std::vector<Tick> getTicks() { return instantMeter.ticks; }
    int getTickYoffset() { return textMeter.getHeight(); }
    
private:
    TextMeter textMeter;
    Meter averageMeter;
    Meter instantMeter;
    
    Averager<float> averager{12, NegativeInfinity};
    
    int channel;
};

struct StereoMeter : juce::Component
{
    StereoMeter(const juce::String& labelText);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void update(const float& inputL, const float& inputR);
    
private:
    MacroMeter macroMeterL{0};
    DbScale dbScale;
    MacroMeter macroMeterR{1};
    
    juce::String label;
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
        
    StereoMeter stereoMeterRms{"RMS"};
    
#if defined(GAIN_TEST_ACTIVE)
    juce::Slider gainSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment gainAttachment{audioProcessor.apvts, "Gain", gainSlider};
#endif
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PFMProject10AudioProcessorEditor)
};
