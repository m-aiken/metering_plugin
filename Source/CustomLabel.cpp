/*
  ==============================================================================

    CustomLabel.cpp
    Created: 2 Apr 2022 1:16:43am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "CustomLabel.h"
#include "MyColours.h"
#define GlobalFont juce::Font(juce::Font::getDefaultMonospacedFontName(), 12.f, 0)

//==============================================================================
CustomLabel::CustomLabel(const juce::String& labelText)
{
    setText(labelText, juce::NotificationType::dontSendNotification);
}

void CustomLabel::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    g.setColour(MyColours::getColour(MyColours::Text));
    g.setFont(GlobalFont);
    g.drawFittedText(getText(),
                     bounds.getX(),
                     bounds.getY(),
                     bounds.getWidth(),
                     bounds.getHeight(),
                     juce::Justification::centred,
                     1);
}
