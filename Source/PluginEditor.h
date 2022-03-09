/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#define MaxDecibels 6.f
#define NegativeInfinity -48.f

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

//==============================================================================
template<typename T>
struct CircularBuffer
{
    using DataType = std::vector<T>;
    
    CircularBuffer(size_t numElements, T initialValue)
    {
        resize(numElements, initialValue);
    }
    
    void resize(size_t s, T fillValue) { buffer.resize(s, fillValue); }
    void clear(T fillValue) { buffer.assign(getSize(), fillValue); }
    
    void write(T t)
    {
        auto idx = writeIndex.load();
        buffer[idx] = t;
        
        // increment writeIndex
        ++idx;
        if ( idx > getSize() - 1 ) // end of container, circle back to start
            idx = 0;
        
        writeIndex = idx;
    }
    
    DataType& getData() { return buffer; }
    
    size_t getReadIndex() const
    {
        // writeIndex was incremented in write() so now points to oldest item
        return writeIndex.load();
    }
    
    size_t getSize() const { return buffer.size(); }
    
private:
    DataType buffer;
    std::atomic<int> writeIndex = 0;
};

//==============================================================================
enum class HistogramTypes
{
    RMS,
    PEAK
};

struct Histogram : juce::Component
{
    Histogram(const juce::String& _label) : label(_label) { }
    void paint(juce::Graphics& g) override;
    void resized() override;
    void update(const float& inputL, const float& inputR);
    
    void setThreshold(const float& threshAsDecibels);
    
    void setView(const int& selectedId);
    
private:
    CircularBuffer<float> circularBufferStacked{776, NegativeInfinity};
    CircularBuffer<float> circularBufferSideBySide{388, NegativeInfinity};
    juce::String label;
    float threshold = 0.f;
    juce::ColourGradient colourGrad;
    
    int viewId;
};

struct HistogramContainer : juce::Component
{
    HistogramContainer();
    void resized() override;
    
    void update(const HistogramTypes& histoType, const float& inputL, const float& inputR);
    void setThreshold(const HistogramTypes& histoType, const float& threshAsDecibels);
    
    void setFlexDirection(const int& selectedId);
    
private:
    Histogram rmsHistogram{"RMS"};
    Histogram peakHistogram{"PEAK"};
    
    juce::FlexBox fb;
};

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
struct CorrelationMeter : juce::Component
{
    CorrelationMeter(double _sampleRate, size_t _blockSize);
    void prepareFilters();
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> paintMeter(const juce::Rectangle<int>& containerBounds, const int& y, const int& height, const float& value);
    void update(juce::AudioBuffer<float>& incomingBuffer);
    
private:
    using FilterType = juce::dsp::FIR::Filter<float>;
    std::array<FilterType, 3> filters;
    
    double sampleRate;
    size_t blockSize;
    
    Averager<float> instantaneousCorrelation{ blockSize, 0.f };
    Averager<float> averagedCorrelation{ blockSize * 6, 0.f };
};

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

//==============================================================================
struct ValueHolderBase : juce::Timer
{
    ValueHolderBase() { startTimerHz(30); }
    ~ValueHolderBase() { stopTimer(); }
    
    void setHoldTime(const long long& ms) { holdTime = ms; }
    long long getHoldTime() { return holdTime; }
    float getCurrentValue() const { return currentValue; }
    float getHeldValue() const { return heldValue; }
    void resetTick() { currentValue = NegativeInfinity; }
    
    void timerCallback() override;
    virtual void handleOverHoldTime() = 0;
    
    friend struct DecayingValueHolder;
    friend struct ValueHolder;
private:
    float currentValue = NegativeInfinity;
    float heldValue = NegativeInfinity;
   
    long long now = juce::Time::currentTimeMillis();
    long long peakTime = 0;
    long long holdTime;
};

//==============================================================================
struct DecayingValueHolder : ValueHolderBase
{
    DecayingValueHolder() { }
    ~DecayingValueHolder() { }
    
    void updateHeldValue(const float& input);
    void setDecayRate(const float& dbPerSecond);
    
    void handleOverHoldTime() override;
    
private:
    int timerFrequency = 30;
    float initDecayRate = 12.f;
    float decayRatePerFrame;
};

//==============================================================================
struct ValueHolder : ValueHolderBase
{
    ValueHolder() { }
    ~ValueHolder() { }
    
    void setThreshold(const float& threshAsDecibels);
    void updateHeldValue(const float& input);
    bool getIsOverThreshold() const { return isOverThreshold; }
    
    void handleOverHoldTime() override;
    
private:
    float threshold = 0.f;
    bool isOverThreshold = false;
};

//==============================================================================
struct TextMeter : juce::Component
{
    void paint(juce::Graphics& g) override;
    void update(const float& input);
    void setThreshold(const float& threshAsDecibels);
    
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
    
    void setThreshold(const float& threshAsDecibels);
    void setDecayRate(const float& dbPerSecond);
    void setHoldTime(const long long& ms);
    void resetTick();
    
    void setTickVisibility(const bool& toggleState);
    
    std::vector<Tick> ticks;
private:
    float level = 0.f;
    
    DecayingValueHolder fallingTick;
    bool fallingTickEnabled;
    
    float threshold = 0.f;
};

//==============================================================================
enum class Channel
{
    Left,
    Right
};

//==============================================================================
struct MacroMeter : juce::Component
{
    MacroMeter(const Channel& channel);

    void resized() override;
    void update(const float& input);
    
    std::vector<Tick> getTicks() { return instantMeter.ticks; }
    int getTickYoffset() { return textMeter.getHeight(); }
    
    void setThreshold(const float& threshAsDecibels);
    void setDecayRate(const float& dbPerSecond);
    void setHoldTime(const long long& ms);
    void resetTick();
    void setMeterView(const int& newViewId);
    void setTickVisibility(const bool& toggleState);
    
private:
    TextMeter textMeter;
    Meter averageMeter;
    Meter instantMeter;
    
    Averager<float> averager{12, NegativeInfinity};
    
    Channel channel;
};

//==============================================================================
struct CustomLookAndFeel : juce::LookAndFeel_V4
{
    void drawLinearSlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPos,
                          float minSliderPos,
                          float maxSliderPos,
                          const juce::Slider::SliderStyle style,
                          juce::Slider& slider) override;
};

//==============================================================================
struct ThresholdSlider : juce::Slider
{
    ThresholdSlider();
    ~ThresholdSlider();
    
    void paint(juce::Graphics& g) override;
    
private:
    CustomLookAndFeel lnf;
};

//==============================================================================
struct StereoMeter : juce::Component
{
    StereoMeter(const juce::String& labelText);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void update(const float& inputL, const float& inputR);
    
    void setThreshold(const float& threshAsDecibels);
    void setDecayRate(const float& dbPerSecond);
    
    void setTickVisibility(const bool& toggleState);
    void setTickHoldTime(const int& selectedId);
    void resetTick();
    void setMeterView(const int& newViewId);
    
    ThresholdSlider threshCtrl;
private:
    MacroMeter macroMeterL{ Channel::Left };
    DbScale dbScale;
    MacroMeter macroMeterR{ Channel::Right };
    
    juce::String label;
    
    float dbScaleLabelCrossover = 0.94f;
    
    CustomLookAndFeel customStyle;
};

//==============================================================================
struct CustomComboBox : juce::ComboBox
{
    CustomComboBox(const juce::StringArray& choices, const int& initSelectionId);
    ~CustomComboBox();
    void paint(juce::Graphics& g) override;
};

struct CustomLabel : juce::Label
{
    CustomLabel(const juce::String& labelText);
    ~CustomLabel();
    void paint(juce::Graphics& g) override;
};

struct CustomToggle : juce::ToggleButton
{
    CustomToggle(const juce::String& buttonText);
    ~CustomToggle();
    void paint(juce::Graphics& g) override;
};

//==============================================================================
struct MeterComboGroup : juce::Component
{
    MeterComboGroup();
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    float getCurrentDecayRate();
    
    juce::StringArray decayRateChoices { "-3dB/s", "-6dB/s", "-12dB/s", "-24dB/s", "-36dB/s" };
    juce::StringArray avgDurationChoices { "100ms", "250ms", "500ms", "1000ms", "2000ms" };
    juce::StringArray meterViewChoices { "Both", "Peak", "Avg" };
    
    CustomComboBox decayRateCombo { decayRateChoices, 3 };
    CustomComboBox avgDurationCombo { avgDurationChoices, 3 };
    CustomComboBox meterViewCombo { meterViewChoices, 1 };
    
private:
    CustomLabel decayRateLabel { "DECAY" };
    CustomLabel avgDurationLabel { "AVG" };
    CustomLabel meterViewLabel { "METER" };
};

//==============================================================================
struct GonioHoldHistGuiControls : juce::Component
{
    GonioHoldHistGuiControls();
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    juce::Slider gonioScaleKnob;
    
    CustomToggle holdButton { "HOLD" };
    juce::StringArray holdTimeChoices {"0s", "0.5s", "2s", "4s", "6s", "inf"};
    CustomComboBox holdTimeCombo { holdTimeChoices, 2 };
    juce::TextButton holdResetButton { "RESET" };
    
    juce::StringArray histViewChoices { "Stacked", "Side-by-Side" };
    CustomComboBox histViewCombo { histViewChoices, 1 };
    
private:
    CustomLabel gonioScaleLabel { "SCALE" };
    CustomLabel histViewLabel { "HIST" };
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
    StereoMeter stereoMeterPeak{"PEAK"};
    
    HistogramContainer histograms;
    
    StereoImageMeter stereoImageMeter;
    
    MeterComboGroup meterCombos;
    GonioHoldHistGuiControls gonioHoldHistControls;
    
#if defined(GAIN_TEST_ACTIVE)
    juce::Slider gainSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment gainAttachment{audioProcessor.apvts, "Gain", gainSlider};
#endif
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PFMProject10AudioProcessorEditor)
};
