/*
  ==============================================================================

    CustomComboBox.cpp
    Created: 2 Apr 2022 1:13:42am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "CustomComboBox.h"

//==============================================================================
CustomComboBox::CustomComboBox(const juce::StringArray& choices)
{
    setLookAndFeel(&lnf);
    addItemList(choices, 1);
}

void CustomComboBox::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    getLookAndFeel().drawComboBox(g,
                                  bounds.getWidth(),  // width
                                  bounds.getHeight(), // height
                                  true,               // isButtonDown
                                  bounds.getX(),      // button x
                                  bounds.getY(),      // button y
                                  bounds.getWidth(),  // button width
                                  bounds.getHeight(), // button height
                                  *this);             // combo box
}
