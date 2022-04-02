/*
  ==============================================================================

    TextMeter.cpp
    Created: 2 Apr 2022 1:01:42am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "TextMeter.h"
#include "MyColours.h"
#define GlobalFont juce::Font(juce::Font::getDefaultMonospacedFontName(), 12.f, 0)

//==============================================================================
void TextMeter::paint(juce::Graphics& g)
{
    juce::String str;
    
    if ( valueHolder.isOverThreshold() )
    {
        str = juce::String(valueHolder.getHeldValue(), 1);
        g.fillAll(MyColours::getColour(MyColours::Red)); // background
    }
    else
    {
        str = juce::String(valueHolder.getCurrentValue(), 1);
    }
    
    g.setColour(MyColours::getColour(MyColours::Text));
    g.setFont(GlobalFont);
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

void TextMeter::setThreshold(const float& threshAsDecibels)
{
    valueHolder.setThreshold(threshAsDecibels);
}
