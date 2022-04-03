/*
  ==============================================================================

    StereoMeter.cpp
    Created: 2 Apr 2022 1:11:51am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "StereoMeter.h"
#include "MyColours.h"
#include "Globals.h"

//==============================================================================
StereoMeter::StereoMeter(const juce::String& labelText)
    : label(labelText)
{
    addAndMakeVisible(macroMeterL);
    addAndMakeVisible(dbScale);
    addAndMakeVisible(macroMeterR);
    addAndMakeVisible(threshCtrl);
}

void StereoMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto h = bounds.getHeight();
    auto w = bounds.getWidth();
    auto labelContainerY = static_cast<int>(h * dbScaleLabelCrossover);
    auto labelContainerH = static_cast<int>(h - labelContainerY);
    
    g.setColour(MyColours::getColour(MyColours::Text));
    g.setFont(Globals::font());
    
    std::vector<juce::String> labels{"L", label, "R"};
    std::vector<int> xPositions{0, static_cast<int>(w / 3), static_cast<int>(w - (w / 3))};
    
    for (auto i = 0; i < labels.size(); ++i)
    {
        g.drawFittedText(labels[i],                                // text
                         xPositions[i],                            // x
                         labelContainerY,                          // y
                         static_cast<int>(w / 3),                  // width
                         labelContainerH,                          // height
                         juce::Justification::horizontallyCentred, // justification
                         1);                                       // max num lines
    }
}

void StereoMeter::resized()
{
    auto bounds = getLocalBounds();
    auto h = bounds.getHeight();
    auto w = bounds.getWidth();
    auto meterWidth = static_cast<int>(w * 0.35f);
    auto meterHeight = static_cast<int>(h * 0.92f);
    
    auto dbScaleWidth = w - (meterWidth * 2);
    
    macroMeterL.setBounds(0, 0, meterWidth, meterHeight);
    
    dbScale.ticks = macroMeterL.getTicks();
    auto tickYoffset = macroMeterL.getTickYoffset();
    dbScale.yOffset = macroMeterL.getY() + tickYoffset;
    dbScale.setBounds(macroMeterL.getRight(),
                      0,
                      dbScaleWidth,
                      static_cast<int>(h * dbScaleLabelCrossover));
    
    macroMeterR.setBounds(dbScale.getRight(), 0, meterWidth, meterHeight);
    
    auto offset = macroMeterL.getTickYoffset();
    threshCtrl.setBounds(dbScale.getX(),
                         macroMeterL.getY() + offset,
                         dbScale.getWidth(),
                         macroMeterL.getHeight() - offset);
}

void StereoMeter::update(const float& inputL, const float& inputR)
{
    macroMeterL.update(inputL);
    macroMeterR.update(inputR);
}

void StereoMeter::setThreshold(const float& threshAsDecibels)
{
    macroMeterL.setThreshold(threshAsDecibels);
    macroMeterR.setThreshold(threshAsDecibels);
}

void StereoMeter::setDecayRate(const int& selectedId)
{
    float dbPerSecond;
    
    switch (selectedId)
    {
        case 1:  dbPerSecond = 3.f;  break;
        case 2:  dbPerSecond = 6.f;  break;
        case 3:  dbPerSecond = 12.f; break;
        case 4:  dbPerSecond = 24.f; break;
        case 5:  dbPerSecond = 36.f; break;
        default: dbPerSecond = 12.f; break;
    }
    
    macroMeterL.setDecayRate(dbPerSecond);
    macroMeterR.setDecayRate(dbPerSecond);
}

void StereoMeter::setTickVisibility(const bool& toggleState)
{
    macroMeterL.setTickVisibility(toggleState);
    macroMeterR.setTickVisibility(toggleState);
}

void StereoMeter::setTickHoldTime(const int& selectedId)
{
    long long holdTimeMs;
    
    switch (selectedId)
    {
        case 1:  holdTimeMs = 0;        break;
        case 2:  holdTimeMs = 500;      break;
        case 3:  holdTimeMs = 2000;     break;
        case 4:  holdTimeMs = 4000;     break;
        case 5:  holdTimeMs = 6000;     break;
        case 6:  holdTimeMs = INFINITY; break;
        default: holdTimeMs = 500;      break;
    }
    
    macroMeterL.setHoldTime(holdTimeMs);
    macroMeterR.setHoldTime(holdTimeMs);
}

void StereoMeter::resetValueHolder()
{
    macroMeterL.resetValueHolder();
    macroMeterR.resetValueHolder();
}

void StereoMeter::setMeterView(const int& newViewId)
{
    macroMeterL.setMeterView(newViewId);
    macroMeterR.setMeterView(newViewId);
}

void StereoMeter::resizeAverager(const int& durationId)
{
    macroMeterL.resizeAverager(durationId);
    macroMeterR.resizeAverager(durationId);
}
