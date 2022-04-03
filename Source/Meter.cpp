/*
  ==============================================================================

    Meter.cpp
    Created: 2 Apr 2022 1:05:20am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "Meter.h"
#include "MyColours.h"
#include "Globals.h"

//==============================================================================
void Meter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto h = bounds.getHeight();
    
    auto shadow = MyColours::getDropShadow();
    shadow.drawForRectangle(g, bounds);
    
    auto overThreshColour = MyColours::getColour(MyColours::Red);

    auto levelJmap = juce::jmap<float>(level, Globals::negInf(), Globals::maxDb(), h, 0);
    auto thrshJmap = juce::jmap<float>(threshold, Globals::negInf(), Globals::maxDb(), h, 0);
    
    auto underThreshGradient = MyColours::getMeterGradient(bounds.getHeight(), bounds.getHeight() / 3, MyColours::GradientOrientation::Vertical);
    
    g.setGradientFill(underThreshGradient);
    if ( threshold <= level )
    {
        g.setColour(overThreshColour);
        g.fillRect(bounds.withHeight((h * levelJmap) - (thrshJmap - 1)).withY(levelJmap));
        
        g.setGradientFill(underThreshGradient);
        g.fillRect(bounds.withHeight(h * (thrshJmap + 1)).withY(thrshJmap + 1));
    }
    else
    {
        g.fillRect(bounds.withHeight(h * levelJmap).withY(levelJmap));
    }
    
    // falling tick
    if ( fallingTickEnabled )
    {
        g.setColour(MyColours::getColour(MyColours::Yellow));
        
        auto tickValue = fallingTick.getHoldTime() == 0 ? level :   fallingTick.getCurrentValue();
        
        auto ftJmap = juce::jmap<float>(tickValue,
                                        Globals::negInf(),
                                        Globals::maxDb(),
                                        h,
                                        0);
        
        g.drawLine(bounds.getX(),     // startX
                   ftJmap,            // startY
                   bounds.getRight(), // endX
                   ftJmap,            // endY
                   3.f);              // line thickness
    }
}

void Meter::resized()
{
    ticks.clear();
    int h = getHeight();
    
    for ( int i = static_cast<int>(Globals::negInf()); i <= static_cast<int>(Globals::maxDb()); ++i ) // <= maxDb to include max value
    {
        Tick tick;
        if ( i % 6 == 0 )
        {
            tick.db = static_cast<float>(i);
            tick.y = juce::jmap<int>(i, Globals::negInf(), Globals::maxDb(), h, 0);
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

void Meter::setThreshold(const float& threshAsDecibels)
{
    threshold = threshAsDecibels;
}

void Meter::setDecayRate(const float& dbPerSecond)
{
    fallingTick.setDecayRate(dbPerSecond);
}

void Meter::setHoldTime(const long long& ms)
{
    fallingTick.setHoldTime(ms);
}

void Meter::resetValueHolder()
{
    fallingTick.reset();
}

void Meter::setTickVisibility(const bool& toggleState)
{
    fallingTickEnabled = toggleState;
    repaint();
}
