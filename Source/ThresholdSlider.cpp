/*
  ==============================================================================

    ThresholdSlider.cpp
    Created: 2 Apr 2022 1:10:42am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "ThresholdSlider.h"
#include "Globals.h"

//==============================================================================
ThresholdSlider::ThresholdSlider()
{
    setLookAndFeel(&lnf);
    setRange(Globals::negInf(), Globals::maxDb());
    setValue(0.f);
}

ThresholdSlider::~ThresholdSlider() { setLookAndFeel(nullptr); }

void ThresholdSlider::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto valueToDraw = juce::jmap<float>(getValue(),
                                         Globals::negInf(),
                                         Globals::maxDb(),
                                         bounds.getBottom(),
                                         bounds.getY());
    
    getLookAndFeel().drawLinearSlider(g,
                                      bounds.getX(),
                                      bounds.getY(),
                                      bounds.getWidth(),
                                      bounds.getHeight(),
                                      valueToDraw,          // sliderPos
                                      bounds.getBottom(),   // minSliderPos
                                      bounds.getY(),        // maxSliderPos
                                      juce::Slider::SliderStyle::LinearVertical,
                                      *this);
}
