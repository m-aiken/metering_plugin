/*
  ==============================================================================

    CustomComboBox.h
    Created: 2 Apr 2022 1:13:42am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

//==============================================================================
struct CustomComboBox : juce::ComboBox
{
    CustomComboBox(const juce::StringArray& choices);
    ~CustomComboBox() { setLookAndFeel(nullptr); }
    void paint(juce::Graphics& g) override;
private:
    CustomLookAndFeel lnf;
};
