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
namespace MyColours
{

enum Palette
{
    Green,
    Red,
    RedBright,
    Text,
    Background,
    Yellow
};

inline std::map<Palette, juce::Colour> colourMap =
{
    { Green,      juce::Colour(89u, 255u, 103u).withAlpha(0.6f) },
    { Red,        juce::Colour(196u, 55u, 55u)                  },
    { RedBright,  juce::Colour(255u, 55u, 55u)                  },
    { Text,       juce::Colour(201u, 209u, 217u)                },
    { Background, juce::Colour(13u, 17u, 23u)                   },
    { Yellow,     juce::Colour(217, 193, 56)                    }
};

inline juce::Colour getColour(Palette c) { return colourMap[c]; }

}

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

enum HistView
{
    rows = 1,
    columns
};

struct Histogram : juce::Component
{
    Histogram(const juce::String& _label) : label(_label) { }
    void paint(juce::Graphics& g) override;
    void resized() override;
    void update(const float& inputL, const float& inputR);
    
    void setThreshold(const float& threshAsDecibels);
    void setView(const HistView& v);
    juce::Value& getThresholdValueObject() { return threshold; }
    
private:
    CircularBuffer<float> circularBuffer{776, NegativeInfinity};
    
    juce::String label;
    juce::Value threshold;
    juce::ColourGradient colourGrad;
    
    HistView view;
};

struct HistogramContainer : juce::Component
{
    HistogramContainer();
    void resized() override;
    
    void update(const HistogramTypes& histoType, const float& inputL, const float& inputR);
    void setThreshold(const HistogramTypes& histoType, const float& threshAsDecibels);
    
    void setView(const HistView& v);
    juce::Value& getThresholdValueObject(const HistogramTypes& histoType);
    
private:
    Histogram rmsHistogram{"RMS"};
    Histogram peakHistogram{"PEAK"};
        
    HistView view;
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
    ValueHolderBase() { startTimerHz(40); }
    ~ValueHolderBase() { stopTimer(); }
    
    void setHoldTime(const juce::int64& ms) { holdTime = ms; }
    juce::int64 getHoldTime() { return holdTime; }
    float getCurrentValue() const { return currentValue; }
    float getHeldValue() const { return heldValue; }
    void reset() { currentValue = NegativeInfinity; }
    
    void timerCallback() override;
    virtual void handleOverHoldTime() = 0;
    
    friend struct DecayingValueHolder;
    friend struct ValueHolder;
private:
    float currentValue = NegativeInfinity;
    float heldValue = NegativeInfinity;
   
    juce::int64 peakTime = getNow();
    juce::int64 holdTime = 2000;
    
    static juce::int64 getNow() { return juce::Time::currentTimeMillis(); }
};

//==============================================================================
struct DecayingValueHolder : ValueHolderBase
{
    DecayingValueHolder() { setDecayRate(initDecayRate); }
    ~DecayingValueHolder() = default;
    
    void updateHeldValue(const float& input);
    void setDecayRate(const float& dbPerSecond);
    void handleOverHoldTime() override;
    
private:
    int timerFrequency = 40;
    float initDecayRate = 12.f;
    float decayRatePerFrame = 0.f;
    float decayRateMultiplier = 1.f;
    
    void resetDecayRateMultiplier() { decayRateMultiplier = 1.f; }
};

//==============================================================================
struct ValueHolder : ValueHolderBase
{
    ValueHolder() { }
    ~ValueHolder() { }
    
    void setThreshold(const float& threshAsDecibels);
    void updateHeldValue(const float& input);
    bool isOverThreshold() const;
    void handleOverHoldTime() override;
    
private:
    float threshold = 0.f;
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
    void resetValueHolder();
    
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
    void resetValueHolder();
    void setMeterView(const int& newViewId);
    void setTickVisibility(const bool& toggleState);
    void resizeAverager(const int& durationId);
    
private:
    TextMeter textMeter;
    Meter averageMeter;
    Meter instantMeter;
    
    Averager<float> averager{20, NegativeInfinity};
    
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
    
    void drawComboBox(juce::Graphics& g,
                      int width, int height,
                      bool isButtonDown,
                      int buttonX, int buttonY,
                      int buttonW, int buttonH,
                      juce::ComboBox& comboBox) override;
    
    void drawToggleButton(juce::Graphics& g,
                          juce::ToggleButton& toggleButton,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;
    
    void drawButtonBackground(juce::Graphics& g,
                              juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;
    
    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPosProportional,
                          float rotaryStartAngle,
                          float rotaryEndAngle,
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

//==============================================================================
struct CustomComboBox : juce::ComboBox
{
    CustomComboBox(const juce::StringArray& choices);
    ~CustomComboBox() { setLookAndFeel(nullptr); }
    void paint(juce::Graphics& g) override;
private:
    CustomLookAndFeel lnf;
};

struct CustomLabel : juce::Label
{
    CustomLabel(const juce::String& labelText);
    void paint(juce::Graphics& g) override;
};

struct CustomToggle : juce::ToggleButton
{
    CustomToggle(const juce::String& buttonText);
    ~CustomToggle() { setLookAndFeel(nullptr); }
    void paint(juce::Graphics& g) override;
private:
    CustomLookAndFeel lnf;
};

struct CustomTextBtn : juce::TextButton
{
    CustomTextBtn(const juce::String& buttonText);
    ~CustomTextBtn() { setLookAndFeel(nullptr); }
    void paint(juce::Graphics& g) override;
    void animateButton();
private:
    CustomLookAndFeel lnf;
    
    bool inClickState = false;
    std::function<void()> resetColour = [this]()
    {
        inClickState = false;
        repaint();
    };
};

struct CustomRotary : juce::Slider
{
    CustomRotary();
    ~CustomRotary() { setLookAndFeel(nullptr); }
    void paint(juce::Graphics& g) override;
private:
    CustomLookAndFeel lnf;
};

//==============================================================================
enum class ToggleGroup
{
    DecayRate,
    AverageTime,
    MeterView,
    HoldTime,
    HistView
};

struct ToggleGroupBase
{
    juce::Grid generateGrid(std::vector<CustomToggle*>& toggles);
};

struct DecayRateToggleGroup : ToggleGroupBase, juce::Component
{
    DecayRateToggleGroup();
    void resized() override;
    juce::Value& getValueObject() { return selectedValue; }
    void setSelectedValue(const int& selectedId) { selectedValue.setValue(selectedId); }
    void setSelectedToggleFromState();
    
    CustomToggle optionA{"-3"}, optionB{"-6"}, optionC{"-12"}, optionD{"-24"}, optionE{"-36"};
    
    std::vector<CustomToggle*> toggles = { &optionA, &optionB, &optionC, &optionD, &optionE };
    
private:
    juce::Value selectedValue;
};

struct AverageTimeToggleGroup : ToggleGroupBase, juce::Component
{
    AverageTimeToggleGroup();
    void resized() override;
    juce::Value& getValueObject() { return selectedValue; }
    void setSelectedValue(const int& selectedId) { selectedValue.setValue(selectedId); }
    void setSelectedToggleFromState();
    
    CustomToggle optionA{"100"}, optionB{"250"}, optionC{"500"}, optionD{"1000"}, optionE{"2000"};
    
    std::vector<CustomToggle*> toggles = { &optionA, &optionB, &optionC, &optionD, &optionE };
    
private:
    juce::Value selectedValue;
};

struct MeterViewToggleGroup : ToggleGroupBase, juce::Component
{
    MeterViewToggleGroup();
    void resized() override;
    juce::Value& getValueObject() { return selectedValue; }
    void setSelectedValue(const int& selectedId) { selectedValue.setValue(selectedId); }
    void setSelectedToggleFromState();
    
    CustomToggle optionA{"Both"}, optionB{"Peak"}, optionC{"Avg"};
    
    std::vector<CustomToggle*> toggles = { &optionA, &optionB, &optionC };
    
private:
    juce::Value selectedValue;
};

struct HoldTimeToggleGroup : ToggleGroupBase, juce::Component
{
    HoldTimeToggleGroup();
    void resized() override;
    juce::Value& getValueObject() { return selectedValue; }
    void setSelectedValue(const int& selectedId) { selectedValue.setValue(selectedId); }
    void setSelectedToggleFromState();
    
    CustomToggle optionA{"0s"}, optionB{"0.5s"}, optionC{"2s"}, optionD{"4s"}, optionE{"6s"}, optionF{"inf"};
    
    std::vector<CustomToggle*> toggles = { &optionA, &optionB, &optionC, &optionD, &optionE, &optionF };
    
private:
    juce::Value selectedValue;
};

struct HistViewToggleGroup : ToggleGroupBase, juce::Component
{
    HistViewToggleGroup();
    void resized() override;
    juce::Value& getValueObject() { return selectedValue; }
    void setSelectedValue(const int& selectedId) { selectedValue.setValue(selectedId); }
    void setSelectedToggleFromState();
    
    CustomToggle optionA{"Rows"}, optionB{"Columns"};
    
    std::vector<CustomToggle*> toggles = { &optionA, &optionB };
    
private:
    juce::Value selectedValue;
};

//==============================================================================
struct LineBreak : juce::Component
{
    void paint(juce::Graphics& g) override;
};

struct GuiControlsGroupA : juce::Component
{
    GuiControlsGroupA();
    void resized() override;
    
    DecayRateToggleGroup decayRate;
    AverageTimeToggleGroup avgDuration;
    MeterViewToggleGroup meterView;
    
private:
    CustomLabel decayRateLabel { "Decay Rate (dB/s)" };
    CustomLabel avgDurationLabel { "Average Duration (ms)" };
    CustomLabel meterViewLabel { "Meter View" };
    
    LineBreak lineBreak1, lineBreak2;
};

//==============================================================================
struct GuiControlsGroupB : juce::Component
{
    GuiControlsGroupB();
    void resized() override;
    
    CustomRotary gonioScaleKnob;
    
    CustomToggle holdButton { "HOLD" };
//    juce::StringArray holdTimeChoices {"0s", "0.5s", "2s", "4s", "6s", "inf"};
    HoldTimeToggleGroup holdTime;
//    CustomComboBox holdTimeCombo { holdTimeChoices };
    CustomTextBtn holdResetButton { "RESET" };
    
    HistViewToggleGroup histView;
    
private:
    CustomLabel gonioScaleLabel { "Scale" };
    CustomLabel histViewLabel { "Histogram View" };
    
    LineBreak lineBreak1, lineBreak2;
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
    
    GuiControlsGroupA toggles;
    GuiControlsGroupB guiControlsB;
    
    void updateParams(const ToggleGroup& toggleGroup, const int& selectedId);
    
#if defined(GAIN_TEST_ACTIVE)
    juce::Slider gainSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment gainAttachment{audioProcessor.apvts, "Gain", gainSlider};
#endif
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PFMProject10AudioProcessorEditor)
};
