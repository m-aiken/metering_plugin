/*
  ==============================================================================

    CustomLookAndFeel.cpp
    Created: 2 Apr 2022 1:09:01am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "CustomLookAndFeel.h"
#include "MyColours.h"

//==============================================================================
void CustomLookAndFeel::drawLinearSlider(juce::Graphics& g,
                                         int x, int y, int width, int height,
                                         float sliderPos,
                                         float minSliderPos,
                                         float maxSliderPos,
                                         const juce::Slider::SliderStyle style,
                                         juce::Slider& slider)
{
    slider.setSliderStyle(style);
    
    auto threshold = juce::Rectangle<float>(x, sliderPos, width, 2.f);
    g.setColour(MyColours::getColour(MyColours::Red));
    g.fillRect(threshold);
}

void CustomLookAndFeel::drawComboBox(juce::Graphics& g,
                                     int width, int height,
                                     bool isButtonDown,
                                     int buttonX, int buttonY,
                                     int buttonW, int buttonH,
                                     juce::ComboBox& comboBox)
{
    g.fillAll(MyColours::getColour(MyColours::Background).contrasting(0.05f));
    comboBox.setColour(juce::ComboBox::textColourId, MyColours::getColour(MyColours::Text));
}

void CustomLookAndFeel::drawToggleButton(juce::Graphics& g,
                                         juce::ToggleButton& toggleButton,
                                         bool shouldDrawButtonAsHighlighted,
                                         bool shouldDrawButtonAsDown)
{
    auto bounds = toggleButton.getLocalBounds();
    
    // background
    g.fillAll(toggleButton.getToggleState()
              ? MyColours::getColour(MyColours::Yellow)
              : MyColours::getColour(MyColours::Background).contrasting(0.05f));
    
    // text colour
    g.setColour(toggleButton.getToggleState()
                ? MyColours::getColour(MyColours::Background).contrasting(0.05f)
                : MyColours::getColour(MyColours::Text));
    
    g.drawFittedText(toggleButton.getButtonText(),
                     bounds.getX(),
                     bounds.getY(),
                     bounds.getWidth(),
                     bounds.getHeight(),
                     juce::Justification::centred,
                     1);
}

void CustomLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                             juce::Button& button,
                                             const juce::Colour& backgroundColour,
                                             bool shouldDrawButtonAsHighlighted,
                                             bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds();
    
    g.fillAll(backgroundColour);
    
    g.setColour(MyColours::getColour(MyColours::Text));
    g.drawFittedText(button.getButtonText(),
                     bounds.getX(),
                     bounds.getY(),
                     bounds.getWidth(),
                     bounds.getHeight(),
                     juce::Justification::centred,
                     1);
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                         int x, int y, int width, int height,
                                         float sliderPosProportional,
                                         float rotaryStartAngle,
                                         float rotaryEndAngle,
                                         juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(x, y, width, height);
    
    g.setColour(MyColours::getColour(MyColours::Background).contrasting(0.05f));
    g.fillEllipse(bounds);
    
    g.setColour(MyColours::getColour(MyColours::Text));
    
    juce::Path p;
    
    auto centre = bounds.getCentre();
    
    juce::Rectangle<float> r;
    r.setLeft(centre.getX() - 2);
    r.setRight(centre.getX() + 2);
    r.setTop(bounds.getY());
    r.setBottom(centre.getY());
    
    p.addRoundedRectangle(r, 2.f);
    
    jassert(rotaryStartAngle < rotaryEndAngle);
    
    auto transform = juce::AffineTransform().rotated(sliderPosProportional,
                                                     centre.getX(),
                                                     centre.getY());
    
    p.applyTransform(transform);
    
    g.fillPath(p);
}
