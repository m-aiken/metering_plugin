/*
  ==============================================================================

    GonioScaleControl.cpp
    Created: 2 Apr 2022 1:46:11am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "GonioScaleControl.h"

//==============================================================================
GonioScaleControl::GonioScaleControl()
{
    addAndMakeVisible(gonioScaleLabel);
    addAndMakeVisible(gonioScaleKnob);
    gonioScaleKnob.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    gonioScaleKnob.setRange(50.0, 200.0);
    gonioScaleKnob.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    gonioScaleKnob.setValue(100.0);
}

void GonioScaleControl::resized()
{
    auto bounds = getLocalBounds();
    auto width = bounds.getWidth();
    auto block = bounds.getHeight() / 4;
    
    gonioScaleLabel.setBounds(0, 0, width, block);
    gonioScaleKnob.setBounds(0, gonioScaleLabel.getBottom(), width, block * 3);
}
