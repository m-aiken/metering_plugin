/*
  ==============================================================================

    Histogram.cpp
    Created: 2 Apr 2022 12:44:53am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "Histogram.h"
#include "MyColours.h"

#define MaxDecibels 6.f
#define NegativeInfinity -48.f
#define GlobalFont juce::Font(juce::Font::getDefaultMonospacedFontName(), 12.f, 0)

//==============================================================================
void Histogram::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto height = bounds.getHeight();
    auto width = bounds.getWidth();
    
    /*
    Filling the entire container with the gradient and threshold colours
    The path to fill is the negative of the buffer data
    That path hides anything not from the proper signal with "Background Colour"
    */
    auto gradient = MyColours::getMeterGradient(bounds.getHeight(), 0, MyColours::GradientOrientation::Vertical);
    
    g.setGradientFill(gradient);
    g.fillAll();

    auto mappedThresh = juce::jmap<float>(threshold.getValue(),
                                          NegativeInfinity,
                                          MaxDecibels,
                                          bounds.getHeight(),
                                          0);
    
    juce::Rectangle<float> redRect (0,
                                    0,
                                    bounds.getWidth(),
                                    mappedThresh);
    
    g.setColour(MyColours::getColour(MyColours::Red));
    g.fillRect(redRect);
    
    auto& data = circularBuffer.getData();
    auto readIdx = circularBuffer.getReadIndex();
    auto bufferSize = circularBuffer.getSize();

    if ( view == HistView::columns )
    {
        readIdx = (readIdx + (bufferSize / 2)) % bufferSize;
    }
    
    g.setColour(MyColours::getColour(MyColours::Background));
    juce::Path p;
    
    // manually setting first and last pixel's column (x) outside of the loops
    p.startNewSubPath(0, 0);
    
    auto x = 1;
    
    for ( auto i = readIdx; i < bufferSize - 1; ++i )
    {
        auto scaledValue = juce::jmap<float>(data[i], NegativeInfinity, MaxDecibels, height, 0);
        
        p.lineTo(x, scaledValue);
        
        ++x;
    }

    for ( auto j = 0; j < readIdx; ++j )
    {
        
        auto scaledValue = juce::jmap<float>(data[j], NegativeInfinity, MaxDecibels, height, 0);
        
        p.lineTo(x, scaledValue);

        ++x;
    }
    
    p.lineTo(bufferSize - 1, 0);
    p.closeSubPath();
    g.fillPath(p);
    
    g.setColour(MyColours::getColour(MyColours::Text));
    g.setFont(GlobalFont);
    g.drawFittedText(label,                           // text
                     bounds.reduced(4),               // area
                     juce::Justification::centredTop, // justification
                     1);                              // max num lines
    
    
    g.setColour(MyColours::getColour(MyColours::Background).contrasting(0.05f));
    // startX, startY, endX, endY, thickness
    g.drawLine(0, 0, 0, height, 2.f);
    g.drawLine(width, 0, width, height, 2.f);
    
    g.drawLine(0, 0, width, 0, 2.f);
    g.drawLine(0, height, width, height, 2.f);
}

void Histogram::update(const float& inputL, const float& inputR)
{
    auto average = (inputL + inputR) / 2;
    circularBuffer.write(average);
    
    repaint();
}

void Histogram::setThreshold(const float& threshAsDecibels)
{
    threshold.setValue(threshAsDecibels);
}

void Histogram::setView(const HistView& v)
{
    view = v;
    repaint();
}
