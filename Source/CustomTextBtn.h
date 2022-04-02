/*
  ==============================================================================

    CustomTextBtn.h
    Created: 2 Apr 2022 1:16:59am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

//==============================================================================
struct CustomTextBtn : juce::TextButton
{
    CustomTextBtn(const juce::String& buttonText);
    ~CustomTextBtn() { setLookAndFeel(nullptr); }
    void paint(juce::Graphics& g) override;
    void animateButton();
private:
    CustomLookAndFeel lnf;
    
    bool inClickState = false;
    std::function<void()> resetColour = [this]()
    {
        inClickState = false;
        repaint();
    };
};
