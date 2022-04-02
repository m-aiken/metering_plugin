/*
  ==============================================================================

    DbScale.cpp
    Created: 2 Apr 2022 1:03:29am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "DbScale.h"
#include "MyColours.h"
#define GlobalFont juce::Font(juce::Font::getDefaultMonospacedFontName(), 12.f, 0)

//==============================================================================
void DbScale::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();

    int textHeight = 12;
    
    g.setColour(MyColours::getColour(MyColours::Text));
    g.setFont(GlobalFont);
    
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
