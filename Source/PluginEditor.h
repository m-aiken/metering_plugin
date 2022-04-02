/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include "StereoMeter.h"
#include "HistogramContainer.h"
#include "StereoImageMeter.h"
#include "HoldResetButtons.h"
#include "TimeControls.h"
#include "GonioScaleControl.h"
#include "ViewControls.h"
#include "ToggleGroup.h"

#define MaxDecibels 6.f
#define NegativeInfinity -48.f
#define GlobalFont juce::Font(juce::Font::getDefaultMonospacedFontName(), 12.f, 0)

//==============================================================================
/**
*/
class PFMProject10AudioProcessorEditor  : public juce::AudioProcessorEditor, juce::Timer
{
public:
    PFMProject10AudioProcessorEditor (PFMProject10AudioProcessor&);
    ~PFMProject10AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void timerCallback() override;
    
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PFMProject10AudioProcessor& audioProcessor;
    
    juce::AudioBuffer<float> incomingBuffer;
        
    StereoMeter stereoMeterRms{"RMS"};
    StereoMeter stereoMeterPeak{"PEAK"};
    
    HistogramContainer histograms;
    
    StereoImageMeter stereoImageMeter;
    
    HoldResetButtons holdResetBtns;
    TimeControls timeToggles;
    
    GonioScaleControl gonioControl;
    ViewControls viewToggles;
    
    void initToggleGroupCallbacks(const ToggleGroup& toggleGroup, const std::vector<CustomToggle*>& togglePtrs);
    
    void updateParams(const ToggleGroup& toggleGroup, const int& selectedId);
    
#if defined(GAIN_TEST_ACTIVE)
    juce::Slider gainSlider;
    juce::AudioProcessorValueTreeState::SliderAttachment gainAttachment{audioProcessor.apvts, "Gain", gainSlider};
#endif
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PFMProject10AudioProcessorEditor)
};
