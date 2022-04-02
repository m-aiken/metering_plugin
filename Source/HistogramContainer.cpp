/*
  ==============================================================================

    HistogramContainer.cpp
    Created: 2 Apr 2022 12:48:00am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "HistogramContainer.h"

//==============================================================================
HistogramContainer::HistogramContainer()
{
    addAndMakeVisible(rmsHistogram);
    addAndMakeVisible(peakHistogram);
}

void HistogramContainer::resized()
{
    juce::FlexBox fb;
    fb.flexDirection = ( view == rows ? juce::FlexBox::Direction::column
                                      : juce::FlexBox::Direction::row);
    
    auto rms = juce::FlexItem(rmsHistogram).withFlex(1.f).withMargin(2.f);
    auto peak = juce::FlexItem(peakHistogram).withFlex(1.f).withMargin(2.f);
    
    fb.items.add(rms);
    fb.items.add(peak);
    
    fb.performLayout(getLocalBounds());
}

void HistogramContainer::update(const HistogramTypes& histoType,
                                const float& inputL,
                                const float& inputR)
{
    if ( histoType == HistogramTypes::RMS )
        rmsHistogram.update(inputL, inputR);
    else
        peakHistogram.update(inputL, inputR);
}

void HistogramContainer::setThreshold(const HistogramTypes& histoType,
                                      const float& threshAsDecibels)
{
    if ( histoType == HistogramTypes::RMS )
        rmsHistogram.setThreshold(threshAsDecibels);
    else
        peakHistogram.setThreshold(threshAsDecibels);
}

void HistogramContainer::setView(const int& selectedId)
{
    auto v = (selectedId == HistView::rows) ? HistView::rows : HistView::columns;
    view = v;
    resized();
    
    rmsHistogram.setView(v);
    peakHistogram.setView(v);
}

juce::Value& HistogramContainer::getThresholdValueObject(const HistogramTypes& histoType)
{
    if ( histoType == HistogramTypes::RMS )
        return rmsHistogram.getThresholdValueObject();
    else
        return peakHistogram.getThresholdValueObject();
}
