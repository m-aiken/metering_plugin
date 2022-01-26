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
    auto textBoxHeight = static_cast<int>(h * 0.06);
    auto meterHeight = h - textBoxHeight;
    
    auto w = bounds.getWidth();
    
    // setBounds args (int x, int y, int width, int height)
    textMeter.setBounds(0, 0, w, textBoxHeight);
    
    auto averageMeterWidth = static_cast<int>(w * 0.8);
    auto instantMeterWidth = static_cast<int>(w * 0.15);
    auto meterPadding = static_cast<int>(w * 0.05);
    
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
    addAndMakeVisible(macroMeterLeft);
    addAndMakeVisible(dbScale);
    addAndMakeVisible(macroMeterRight);
}

void StereoMeter::resized()
{
//    auto bounds = getLocalBounds();
    // setBounds args (int x, int y, int width, int height)
    macroMeterLeft.setBounds(0, 20, 40, 320);
    
    dbScale.ticks = macroMeterLeft.getTicks();
    dbScale.yOffset = macroMeterLeft.getY() + macroMeterLeft.getTickYoffset();
    dbScale.setBounds(macroMeterLeft.getRight(), 0, 30, 350);
    
    macroMeterRight.setBounds(dbScale.getRight(), 20, 40, 320);
}

void StereoMeter::update(const float& inputLeft, const float& inputRight)
{
    macroMeterLeft.update(inputLeft);
    macroMeterRight.update(inputRight);
}

//==============================================================================
PFMProject10AudioProcessorEditor::PFMProject10AudioProcessorEditor (PFMProject10AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    startTimerHz(30);
    
//    addAndMakeVisible(macroMeter);
//    addAndMakeVisible(dbScale);
    
    addAndMakeVisible(stereoMeterRms);
    
#if defined(GAIN_TEST_ACTIVE)
    addAndMakeVisible(gainSlider);
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
#endif

    setSize (600, 400);
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
    
    // setBounds args (int x, int y, int width, int height)
//    macroMeter.setBounds(20, 20, 40, 320);
//
//    dbScale.ticks = macroMeter.getTicks();
//    dbScale.yOffset = macroMeter.getY() + macroMeter.getTickYoffset();
//    dbScale.setBounds(macroMeter.getRight(), 0, 30, 350);
    
    stereoMeterRms.setBounds(20, 0, 110, 350);
    
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
        
//        auto rms = incomingBuffer.getRMSLevel(0, 0, incomingBuffer.getNumSamples());
//        auto rmsDb = juce::Decibels::gainToDecibels(rms, NegativeInfinity);

//        macroMeter.update(rmsDb);
        
        auto rmsLeft = incomingBuffer.getRMSLevel(0, // channel
                                                  0,
                                                  incomingBuffer.getNumSamples());
        
        auto rmsRight = incomingBuffer.getRMSLevel(1,
                                                   0,
                                                   incomingBuffer.getNumSamples());
        
        auto rmsDbLeft = juce::Decibels::gainToDecibels(rmsLeft, NegativeInfinity);
        
        auto rmsDbRight = juce::Decibels::gainToDecibels(rmsRight, NegativeInfinity);
        
        stereoMeterRms.update(rmsDbLeft, rmsDbRight);
    }
}
