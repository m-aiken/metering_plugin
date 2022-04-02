/*
  ==============================================================================

    CustomToggle.cpp
    Created: 2 Apr 2022 1:16:50am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "CustomToggle.h"

#define GlobalFont juce::Font(juce::Font::getDefaultMonospacedFontName(), 12.f, 0)

//==============================================================================
CustomToggle::CustomToggle(const juce::String& buttonText)
{
    setLookAndFeel(&lnf);
    setButtonText(buttonText);
}

void CustomToggle::paint(juce::Graphics& g)
{
    g.setFont(GlobalFont);
    getLookAndFeel().drawToggleButton(g,
                                      *this, // toggle button
                                      true,  // draw as highlighted
                                      true); // draw as down
}
