/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

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
    
    auto meterHeight = 300;
    
    // setBounds args (int x, int y, int width, int height)
    monoMeter.setBounds(20, 20, 30, meterHeight);
    
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
    }
}
