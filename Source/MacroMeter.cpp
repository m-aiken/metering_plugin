/*
  ==============================================================================

    MacroMeter.cpp
    Created: 2 Apr 2022 1:07:02am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "MacroMeter.h"

//==============================================================================
MacroMeter::MacroMeter(const Channel& channel)
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
    
    auto averageMeterWidth = static_cast<int>(w * 0.7f);
    auto instantMeterWidth = static_cast<int>(w * 0.25f);
    auto meterPadding = static_cast<int>(w * 0.05f);
    
    auto avgMeterX = (channel == Channel::Left ? 0 : instantMeterWidth + meterPadding);
    
    auto averageMeterRect = juce::Rectangle<int>(avgMeterX,
                                                 textMeter.getBottom(),
                                                 averageMeterWidth,
                                                 meterHeight);
    
    auto instMeterX = channel == Channel::Left ? averageMeterWidth + meterPadding : 0;
    
    auto instantMeterRect = juce::Rectangle<int>(instMeterX,
                                                 textMeter.getBottom(),
                                                 instantMeterWidth,
                                                 meterHeight);
    
    if (channel == Channel::Left)
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

void MacroMeter::setThreshold(const float& threshAsDecibels)
{
    averageMeter.setThreshold(threshAsDecibels);
    instantMeter.setThreshold(threshAsDecibels);
    textMeter.setThreshold(threshAsDecibels);
}

void MacroMeter::setDecayRate(const float& dbPerSecond)
{
    averageMeter.setDecayRate(dbPerSecond);
    instantMeter.setDecayRate(dbPerSecond);
}

void MacroMeter::setHoldTime(const long long& ms)
{
    averageMeter.setHoldTime(ms);
    instantMeter.setHoldTime(ms);
}

void MacroMeter::resetValueHolder()
{
    averageMeter.resetValueHolder();
    instantMeter.resetValueHolder();
}

void MacroMeter::setMeterView(const int& newViewId)
{
    if ( newViewId == 1 ) // Both
    {
        averageMeter.setVisible(true);
        instantMeter.setVisible(true);
    }
    else if ( newViewId == 2 ) // Peak
    {
        averageMeter.setVisible(false);
        instantMeter.setVisible(true);
    }
    else if ( newViewId == 3 ) // Average
    {
        averageMeter.setVisible(true);
        instantMeter.setVisible(false);
    }
}

void MacroMeter::setTickVisibility(const bool& toggleState)
{
    averageMeter.setTickVisibility(toggleState);
    instantMeter.setTickVisibility(toggleState);
}

void MacroMeter::resizeAverager(const int& durationId)
{
    // timerCallback called every 25ms
    size_t newSize;
    
    switch (durationId)
    {
        case 1:  newSize = 4;  break; // 100ms
        case 2:  newSize = 10; break; // 250ms
        case 3:  newSize = 20; break; // 500ms
        case 4:  newSize = 40; break; // 1000ms
        case 5:  newSize = 80; break; // 2000ms
        default: newSize = 20; break; // 500ms
    }
    
    averager.resize(newSize, averager.getAverage());
}
