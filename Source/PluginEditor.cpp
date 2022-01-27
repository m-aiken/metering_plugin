/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
void ValueHolderBase::timerCallback()
{
    now = juce::Time::currentTimeMillis();
    if ( now - peakTime > holdTime )
        handleOverHoldTime();
}

//==============================================================================
void DecayingValueHolder::updateHeldValue(const float& input)
{
    if (input > currentValue)
    {
        currentValue = input;
        peakTime = juce::Time::currentTimeMillis();
    }
}

void DecayingValueHolder::setDecayRate(const float& dbPerSecond)
{
    decayRatePerFrame = dbPerSecond / timerFrequency;
}

void DecayingValueHolder::handleOverHoldTime()
{
    currentValue = juce::jlimit(NegativeInfinity,
                                MaxDecibels,
                                currentValue - decayRatePerFrame);
        
    if ( currentValue == NegativeInfinity )
        setDecayRate(initDecayRate); // reset decayRatePerFrame
    else
        decayRatePerFrame *= 1.04f;
}

//==============================================================================
void ValueHolder::updateHeldValue(const float& input)
{
    currentValue = input;
    
    if (input > mThreshold)
    {
        isOverThreshold = true;
        peakTime = juce::Time::currentTimeMillis();
        if (input > heldValue)
            heldValue = input;
    }
}

void ValueHolder::handleOverHoldTime()
{
    isOverThreshold = currentValue > mThreshold;
    heldValue = NegativeInfinity;
}

//==============================================================================
void TextMeter::paint(juce::Graphics& g)
{
    juce::String str;
    
    if ( valueHolder.getIsOverThreshold() )
    {
        str = juce::String(valueHolder.getHeldValue(), 1);
        g.fillAll(juce::Colour(230u, 33u, 51u)); // red background
    }
    else
    {
        str = juce::String(valueHolder.getCurrentValue(), 1);
    }
    
    g.setColour(juce::Colour(201u, 209u, 217u)); // text colour
    g.setFont(14.0f);
    g.drawFittedText(str,                                      // text
                     getLocalBounds(),                         // area
                     juce::Justification::horizontallyCentred, // justification
                     1);                                       // max num lines
}

void TextMeter::update(const float& input)
{
    valueHolder.updateHeldValue(input);
    repaint();
}

//==============================================================================
void DbScale::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    int textHeight = 8;
    
    g.setColour(juce::Colour(201u, 209u, 217u)); // text colour
    
    for ( int i = 0; i < ticks.size(); ++i)
    {
        auto dbStr = juce::String(ticks[i].db);
        
        g.drawFittedText((ticks[i].db > 0 ? '+' + dbStr : dbStr),   // text
                         bounds.getX(),                             // x
                         ticks[i].y + (yOffset - (textHeight / 2)), // y
                         bounds.getWidth(),                         // width
                         textHeight,                                // height
                         juce::Justification::horizontallyCentred,  // justification
                         1);                                        // maxLines
    }
}

//==============================================================================
void Meter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto h = bounds.getHeight();
    
    g.setColour(juce::Colours::lightgrey); // background colour
    g.fillRect(bounds);
    
    g.setColour(juce::Colours::limegreen); // meter colour
    auto jmap = juce::jmap<float>(level, NegativeInfinity, MaxDecibels, h, 0);
    g.fillRect(bounds.withHeight(h * jmap).withY(jmap));
    
    // falling tick
    auto yellow = juce::Colour(217, 193, 56);
    g.setColour(yellow);
    
    auto ftJmap = juce::jmap<float>(fallingTick.getCurrentValue(),
                                    NegativeInfinity,
                                    MaxDecibels,
                                    h,
                                    0);
    
    g.drawLine(bounds.getX(),     // startX
               ftJmap,            // startY
               bounds.getRight(), // endX
               ftJmap,            // endY
               3.f);              // line thickness
}

void Meter::resized()
{
    ticks.clear();
    int h = getHeight();
    
    for ( int i = static_cast<int>(NegativeInfinity); i <= static_cast<int>(MaxDecibels); ++i ) // <= maxDb to include max value
    {
        Tick tick;
        if ( i % 6 == 0 )
        {
            tick.db = static_cast<float>(i);
            tick.y = juce::jmap<int>(i, NegativeInfinity, MaxDecibels, h, 0);
            ticks.push_back(tick);
        }
    }
}

void Meter::update(const float& newLevel)
{
    level = newLevel;
    fallingTick.updateHeldValue(newLevel);
    repaint();
}

//==============================================================================
MacroMeter::MacroMeter(const int& channel)
    : channel(channel)
{
    addAndMakeVisible(textMeter);
    addAndMakeVisible(averageMeter);
    addAndMakeVisible(instantMeter);
}

void MacroMeter::resized()
{
    auto bounds = getLocalBounds();
    auto h = bounds.getHeight();
    auto textBoxHeight = static_cast<int>(h * 0.06f);
    auto meterHeight = h - textBoxHeight;
    
    auto w = bounds.getWidth();
    
    // setBounds args (int x, int y, int width, int height)
    textMeter.setBounds(0, 0, w, textBoxHeight);
    
    auto averageMeterWidth = static_cast<int>(w * 0.75f);
    auto instantMeterWidth = static_cast<int>(w * 0.20f);
    auto meterPadding = static_cast<int>(w * 0.05f);
    
    auto avgMeterX = (channel == 0 ? 0 : instantMeterWidth + meterPadding);
    
    auto averageMeterRect = juce::Rectangle<int>(avgMeterX,
                                                 textMeter.getBottom(),
                                                 averageMeterWidth,
                                                 meterHeight);
    
    auto instMeterX = channel == 0 ? averageMeterWidth + meterPadding : 0;
    
    auto instantMeterRect = juce::Rectangle<int>(instMeterX,
                                                 textMeter.getBottom(),
                                                 instantMeterWidth,
                                                 meterHeight);
    
    if (channel == 0)
    {
        averageMeter.setBounds(averageMeterRect);
        instantMeter.setBounds(instantMeterRect);
    }
    else
    {
        instantMeter.setBounds(instantMeterRect);
        averageMeter.setBounds(averageMeterRect);
    }
}

void MacroMeter::update(const float& input)
{
    averager.add(input);
    
    textMeter.update(input);
    averageMeter.update(averager.getAverage());
    instantMeter.update(input);
}

//==============================================================================
StereoMeter::StereoMeter(const juce::String& labelText)
    : label(labelText)
{
    addAndMakeVisible(macroMeterL);
    addAndMakeVisible(dbScale);
    addAndMakeVisible(macroMeterR);
}

void StereoMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto h = bounds.getHeight();
    auto w = bounds.getWidth();
    auto labelContainerY = static_cast<int>(h * dbScaleLabelCrossover);
    auto labelContainerH = static_cast<int>(h - labelContainerY);
    
    g.setColour(juce::Colour(201u, 209u, 217u)); // text colour
    
    std::vector<juce::String> labels{"L", label, "R"};
    std::vector<int> xPositions{0, static_cast<int>(w / 3), static_cast<int>(w - (w / 3))};
    
    for (auto i = 0; i < labels.size(); ++i)
    {
        g.drawFittedText(labels[i],                                // text
                         xPositions[i],                            // x
                         labelContainerY,                          // y
                         static_cast<int>(w / 3),                  // width
                         labelContainerH,                          // height
                         juce::Justification::horizontallyCentred, // justification
                         1);                                       // max num lines
    }
}

void StereoMeter::resized()
{
    auto bounds = getLocalBounds();
    auto h = bounds.getHeight();
    auto w = bounds.getWidth();
    auto meterWidth = static_cast<int>(w * 0.35f);
    auto meterHeight = static_cast<int>(h * 0.92f);
    
    auto dbScaleWidth = w - (meterWidth * 2);
    
    macroMeterL.setBounds(0, 0, meterWidth, meterHeight);
    
    dbScale.ticks = macroMeterL.getTicks();
    dbScale.yOffset = macroMeterL.getY() + macroMeterL.getTickYoffset();
    dbScale.setBounds(macroMeterL.getRight(),
                      0,
                      dbScaleWidth,
                      static_cast<int>(h * dbScaleLabelCrossover));
    
    macroMeterR.setBounds(dbScale.getRight(), 0, meterWidth, meterHeight);
}

void StereoMeter::update(const float& inputL, const float& inputR)
{
    macroMeterL.update(inputL);
    macroMeterR.update(inputR);
}

//==============================================================================
PFMProject10AudioProcessorEditor::PFMProject10AudioProcessorEditor (PFMProject10AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    startTimerHz(30);
        
    addAndMakeVisible(stereoMeterRms);
    addAndMakeVisible(stereoMeterPeak);
    
#if defined(GAIN_TEST_ACTIVE)
    addAndMakeVisible(gainSlider);
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
#endif

    setSize (800, 600);
}

PFMProject10AudioProcessorEditor::~PFMProject10AudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void PFMProject10AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour(13u, 17u, 23u));
}

void PFMProject10AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    auto width = bounds.getWidth();
    auto margin = 10;
    auto stereoMeterWidth = 100;
    auto stereoMeterHeight = 350;
    // setBounds args (int x, int y, int width, int height)
    stereoMeterRms.setBounds(margin,
                             margin,
                             stereoMeterWidth,
                             stereoMeterHeight);
    
    stereoMeterPeak.setBounds(width - (stereoMeterWidth + margin),
                              margin,
                              stereoMeterWidth,
                              stereoMeterHeight);
    
#if defined(GAIN_TEST_ACTIVE)
    gainSlider.setBounds(dbScale.getRight(), monoMeter.getY() - 10, 20, meterHeight + 20);
#endif
}

void PFMProject10AudioProcessorEditor::timerCallback()
{
    if ( audioProcessor.fifo.getNumAvailable() > 0 )
    {
        while ( audioProcessor.fifo.pull(incomingBuffer) )
        {
            // do nothing else - just looping through until incomingBuffer = most recent available buffer
        }
        
        auto numSamples = incomingBuffer.getNumSamples();
                
        auto rmsL = incomingBuffer.getRMSLevel(0, 0, numSamples);
        auto rmsR = incomingBuffer.getRMSLevel(1, 0, numSamples);
        auto rmsDbL = juce::Decibels::gainToDecibels(rmsL, NegativeInfinity);
        auto rmsDbR = juce::Decibels::gainToDecibels(rmsR, NegativeInfinity);
        stereoMeterRms.update(rmsDbL, rmsDbR);
        
        auto peakL = incomingBuffer.getMagnitude(0, 0, numSamples);
        auto peakR = incomingBuffer.getMagnitude(1, 0, numSamples);
        auto peakDbL = juce::Decibels::gainToDecibels(peakL, NegativeInfinity);
        auto peakDbR = juce::Decibels::gainToDecibels(peakR, NegativeInfinity);
        stereoMeterPeak.update(peakDbL, peakDbR);
    }
}
