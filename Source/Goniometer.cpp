/*
  ==============================================================================

    Goniometer.cpp
    Created: 2 Apr 2022 12:33:09am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "Goniometer.h"
#include "MyColours.h"
#include "Globals.h"

//==============================================================================
void Goniometer::paint(juce::Graphics& g)
{
    using namespace juce;
    
    auto bounds = getLocalBounds();
    auto centre = bounds.getCentre();
    auto width = bounds.getWidth();
    auto padding = width / 10;
    auto diameter = width - (padding * 2);
    auto radius = diameter / 2;
    
    g.drawImage(this->canvas, bounds.toFloat());
    
    g.setColour(MyColours::getColour(MyColours::GoniometerPath));
    
    Path p;
    
    auto numSamples = buffer.getNumSamples();

    for (auto i = 0; i < numSamples; ++i)
    {
        auto left = buffer.getSample(0, i);
        auto right = buffer.getSample(1, i);

        auto side = jlimit<float>(-1.f, 1.f, (left - right) * Decibels::decibelsToGain(-3.f));
        
        auto mid = jlimit<float>(-1.f, 1.f, (left + right) * Decibels::decibelsToGain(-3.f));
        
        Point<float> point(centre.getX() + radius * scale * side,
                           centre.getY() + radius * scale * mid);
        
        if ( i == 0 )
            p.startNewSubPath(point);
        else
            p.lineTo(point);
    }
    
    g.strokePath(p, PathStrokeType(1.f));
}

void Goniometer::resized()
{
    using namespace juce;
    
    auto bounds = getLocalBounds();
    auto centre = bounds.getCentre();
    auto height = bounds.getHeight();
    auto width = bounds.getWidth();
    auto padding = width / 10;
    auto diameter = width - (padding * 2);
    
    auto backgroundColour = MyColours::getColour(MyColours::Background);
    auto textColour = MyColours::getColour(MyColours::Text);
    auto ellipseColour = textColour.withAlpha(0.1f);
    auto lineColour = textColour.withAlpha(0.025f);
    
    canvas = Image(Image::RGB, width, height, true);
    
    Graphics g (canvas);
    
    // inner lines
    Path linePath;
    Rectangle<float> lineRect;
    lineRect.setLeft(centre.getX() - 1);
    lineRect.setRight(centre.getX() + 1);
    lineRect.setTop(padding);
    lineRect.setBottom(centre.getY());
    linePath.addRectangle(lineRect);
    
    Path labelPath;
    Rectangle<float> labelRect;
    labelRect.setLeft(centre.getX() - (padding / 2));
    labelRect.setRight(centre.getX() + (padding / 2));
    labelRect.setTop(0);
    labelRect.setBottom(padding);
    labelPath.addRectangle(labelRect);
    
    float angle;
    
    for ( auto i = 0; i < 8; ++i)
    {
        angle = degreesToRadians( i * 45.f );
        auto affineT = AffineTransform().rotated(angle, centre.getX(), centre.getY());
        
        // lines
        linePath.applyTransform(affineT);
        g.setColour(lineColour);
        g.fillPath(linePath);
        
        // label boxes
        labelPath.applyTransform(affineT);
        g.setColour(backgroundColour);
        g.fillPath(labelPath);
    }
    
    // draw labels separately - if drawn in the same loop as lines and text boxes 90 degrees doesn't draw??
    std::vector<String> labels { "M", "R", "-S", "", "", "", "+S", "L" };
    g.setColour(textColour);
    g.setFont(Globals::font());
    for ( auto i = 0; i < 8; ++i)
    {
        angle = degreesToRadians( i * 45.f );
        auto affineT = AffineTransform().rotated(angle, centre.getX(), centre.getY());
        
        g.drawText(labels[i], labelRect.transformedBy(affineT), Justification::centred);
    }
    
    // draw ellipse last so that it overlaps label rectanges
    g.setColour(ellipseColour);
    
    g.drawEllipse(padding,  // x
                  padding,  // y
                  diameter, // width
                  diameter, // height
                  2.f);     // line thickness
}

void Goniometer::update(juce::AudioBuffer<float>& incomingBuffer)
{
    if ( incomingBuffer.getNumSamples() >= 265 )
        buffer = incomingBuffer;
    else
        buffer.applyGain(juce::Decibels::decibelsToGain(-3.f));
    
    repaint();
}

void Goniometer::setScale(const double& rotaryValue)
{
    scale = juce::jmap<double>(rotaryValue, 50.0, 200.0, 0.2, 0.8);
    repaint();
}
