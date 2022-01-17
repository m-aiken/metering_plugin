/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ValueHolder::ValueHolder() { startTimerHz(30); }
ValueHolder::~ValueHolder() { stopTimer(); }

void ValueHolder::setThreshold(const float& threshold) { mThreshold = threshold; }

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

void ValueHolder::setHoldTime(const long long& ms) { holdTime = ms; }
float ValueHolder::getCurrentValue() const { return currentValue; }
float ValueHolder::getHeldValue() const { return heldValue; }
bool ValueHolder::getIsOverThreshold() const { return isOverThreshold; }

void ValueHolder::timerCallback()
{
    now = juce::Time::currentTimeMillis();
    if ( now - peakTime > holdTime )
    {
        isOverThreshold = currentValue > mThreshold ? true : false;
        heldValue = NegativeInfinity;
    }
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
    repaint();
}

//==============================================================================
PFMProject10AudioProcessorEditor::PFMProject10AudioProcessorEditor (PFMProject10AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    startTimerHz(30);
    
    addAndMakeVisible(textMeter);
    addAndMakeVisible(monoMeter);
    addAndMakeVisible(dbScale);
    
    
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
    textMeter.setBounds(20, 20, 40, 20);
    
    auto meterHeight = 300;
    monoMeter.setBounds(20, textMeter.getBottom(), 40, meterHeight);
    
    dbScale.ticks = monoMeter.ticks;
    dbScale.yOffset = monoMeter.getY();
    dbScale.setBounds(monoMeter.getRight(), 0, 30, 350);
    
    
    
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
        
        auto rms = incomingBuffer.getRMSLevel(0, 0, incomingBuffer.getNumSamples());
        auto rmsDb = juce::Decibels::gainToDecibels(rms, NegativeInfinity);
        monoMeter.update(rmsDb);
        
//        valHolder.updateHeldValue(rmsDb);
        textMeter.update(rmsDb);
    }
}
