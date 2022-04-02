/*
  ==============================================================================

    CustomTextBtn.cpp
    Created: 2 Apr 2022 1:16:59am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "CustomTextBtn.h"
#include "MyColours.h"
#define GlobalFont juce::Font(juce::Font::getDefaultMonospacedFontName(), 12.f, 0)

//==============================================================================
CustomTextBtn::CustomTextBtn(const juce::String& buttonText)
{
    setLookAndFeel(&lnf);
    setButtonText(buttonText);
}

void CustomTextBtn::paint(juce::Graphics& g)
{
    auto buttonColour = (inClickState
                         ? MyColours::getColour(MyColours::RedBright)
                         : MyColours::getColour(MyColours::Red));
    
    g.setFont(GlobalFont);
    
    getLookAndFeel().drawButtonBackground(g,
                                          *this,        // button
                                          buttonColour, // colour
                                          true,         // draw as highlighted
                                          false);       // draw as down
}

void CustomTextBtn::animateButton()
{
    inClickState = true;
    repaint();
    
    juce::Timer::callAfterDelay(100, resetColour);
}
