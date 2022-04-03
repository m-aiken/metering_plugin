/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PFMProject10AudioProcessorEditor::PFMProject10AudioProcessorEditor (PFMProject10AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), stereoImageMeter(p.getSampleRate(), p.getBlockSize())
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    startTimerHz(40);
    
    addAndMakeVisible(stereoMeterRms);
    addAndMakeVisible(stereoMeterPeak);
    addAndMakeVisible(histograms);
    addAndMakeVisible(stereoImageMeter);
    addAndMakeVisible(holdResetBtns);
    addAndMakeVisible(timeToggles);
    addAndMakeVisible(gonioControl);
    addAndMakeVisible(viewToggles);
    
    auto& state = audioProcessor.valueTree;
    
    // link widgets to valueTree
    holdResetBtns.holdButton.getToggleStateValue().referTo(state.getPropertyAsValue("EnableHold", nullptr));
    
    timeToggles.decayRate.getValueObject().referTo(state.getPropertyAsValue("DecayTime", nullptr));
    timeToggles.avgDuration.getValueObject().referTo(state.getPropertyAsValue("AverageTime", nullptr));
    timeToggles.holdTime.getValueObject().referTo(state.getPropertyAsValue("HoldTime", nullptr));
    
    gonioControl.gonioScaleKnob.getValueObject().referTo(state.getPropertyAsValue("GoniometerScale", nullptr));
    viewToggles.meterView.getValueObject().referTo(state.getPropertyAsValue("MeterViewMode", nullptr));
    viewToggles.histView.getValueObject().referTo(state.getPropertyAsValue("HistogramView", nullptr));
    
    stereoMeterRms.threshCtrl.getValueObject().referTo(state.getPropertyAsValue("RMSThreshold", nullptr));
    stereoMeterPeak.threshCtrl.getValueObject().referTo(state.getPropertyAsValue("PeakThreshold", nullptr));
    
    histograms.getThresholdValueObject(HistogramTypes::RMS).referTo(state.getPropertyAsValue("RMSThreshold", nullptr));
    histograms.getThresholdValueObject(HistogramTypes::PEAK).referTo(state.getPropertyAsValue("PeakThreshold", nullptr));
    
    // set initial values
    bool holdButtonState = state.getPropertyAsValue("EnableHold", nullptr).getValue();
    stereoMeterRms.setTickVisibility(holdButtonState);
    stereoMeterPeak.setTickVisibility(holdButtonState);
    
    updateParams(ToggleGroup::DecayRate, state.getPropertyAsValue("DecayTime", nullptr).getValue());
    timeToggles.decayRate.setSelectedToggleFromState();
    
    updateParams(ToggleGroup::AverageTime, state.getPropertyAsValue("AverageTime", nullptr).getValue());
    timeToggles.avgDuration.setSelectedToggleFromState();
    
    updateParams(ToggleGroup::HoldTime, state.getPropertyAsValue("HoldTime", nullptr).getValue());
    timeToggles.holdTime.setSelectedToggleFromState();
    
    double gonioScale = state.getPropertyAsValue("GoniometerScale", nullptr).getValue();
    stereoImageMeter.setGoniometerScale(gonioScale);
    
    updateParams(ToggleGroup::MeterView, state.getPropertyAsValue("MeterViewMode", nullptr).getValue());
    viewToggles.meterView.setSelectedToggleFromState();
    
    updateParams(ToggleGroup::HistView, state.getPropertyAsValue("HistogramView", nullptr).getValue());
    viewToggles.histView.setSelectedToggleFromState();
    
    float rmsThresh = state.getPropertyAsValue("RMSThreshold", nullptr).getValue();
    stereoMeterRms.setThreshold(rmsThresh);
    histograms.setThreshold(HistogramTypes::RMS, rmsThresh);
    
    float peakThresh = state.getPropertyAsValue("PeakThreshold", nullptr).getValue();
    stereoMeterPeak.setThreshold(peakThresh);
    histograms.setThreshold(HistogramTypes::PEAK, peakThresh);
    
    // handle change events
    stereoMeterRms.threshCtrl.onValueChange = [this]
    {
        stereoMeterRms.setThreshold(stereoMeterRms.threshCtrl.getValue());
    };
    
    stereoMeterPeak.threshCtrl.onValueChange = [this]
    {
        stereoMeterPeak.setThreshold(stereoMeterPeak.threshCtrl.getValue());
    };
    
    holdResetBtns.holdButton.onClick = [this]
    {
        auto toggleState = holdResetBtns.holdButton.getToggleState();
        stereoMeterRms.setTickVisibility(toggleState);
        stereoMeterPeak.setTickVisibility(toggleState);
        
        auto resetIsVisible = holdResetBtns.resetButton.isVisible();
        auto holdTimeId = timeToggles.holdTime.getValueObject().getValue();
        if ( !toggleState && resetIsVisible )
        {
            holdResetBtns.resetButton.setVisible(false);
        }
        else if ( toggleState && static_cast<int>(holdTimeId) == 6 && !resetIsVisible )
        {
            holdResetBtns.resetButton.setVisible(true);
        }
    };
    
    holdResetBtns.resetButton.onClick = [this]
    {
        stereoMeterRms.resetValueHolder();
        stereoMeterPeak.resetValueHolder();
        holdResetBtns.resetButton.animateButton();
    };
    
    gonioControl.gonioScaleKnob.onValueChange = [this]
    {
        auto rotaryValue = gonioControl.gonioScaleKnob.getValue();
        stereoImageMeter.setGoniometerScale(rotaryValue);
    };
    
    initToggleGroupCallbacks(ToggleGroup::DecayRate,   timeToggles.decayRate.toggles);
    initToggleGroupCallbacks(ToggleGroup::AverageTime, timeToggles.avgDuration.toggles);
    initToggleGroupCallbacks(ToggleGroup::HoldTime,    timeToggles.holdTime.toggles);
    initToggleGroupCallbacks(ToggleGroup::MeterView,   viewToggles.meterView.toggles);
    initToggleGroupCallbacks(ToggleGroup::HistView,    viewToggles.histView.toggles);
    
#if defined(GAIN_TEST_ACTIVE)
    addAndMakeVisible(gainSlider);
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
#endif

    setSize (800, 600);
}

PFMProject10AudioProcessorEditor::~PFMProject10AudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void PFMProject10AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(MyColours::getColour(MyColours::Background));
}

void PFMProject10AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    auto width = bounds.getWidth();
    auto padding = 10;
    auto stereoMeterWidth = 90;
    auto stereoMeterHeight = 350;
    
    // setBounds args (int x, int y, int width, int height)
    stereoMeterRms.setBounds(padding,
                             padding,
                             stereoMeterWidth,
                             stereoMeterHeight);
    
    stereoMeterPeak.setBounds(width - (stereoMeterWidth + padding),
                              padding,
                              stereoMeterWidth,
                              stereoMeterHeight);
    
    histograms.setBounds(padding,
                         stereoMeterRms.getBottom() + (padding * 2),
                         width - (padding * 2),
                         210);
    
    auto stereoImageMeterWidth = 280; // this will also be the height of the goniometer
    auto stereoImageMeterHeight = 300;
    
    stereoImageMeter.setBounds(bounds.getCentreX() - (stereoImageMeterWidth / 2),
                              (histograms.getY() / 2) - (stereoImageMeterHeight / 2),
                              stereoImageMeterWidth,
                              stereoImageMeterHeight);
    
    auto comboWidth = stereoImageMeter.getX() - stereoMeterRms.getRight() - (padding * 4);
    
    auto toggleContainerHeight = 300;
    timeToggles.setBounds(stereoMeterRms.getRight() + (padding * 2),
                          stereoImageMeter.getBottom() - toggleContainerHeight,
                          comboWidth,
                          toggleContainerHeight);
    
    auto btnHeight = timeToggles.getY() - (padding * 2);
    holdResetBtns.setBounds(timeToggles.getX(),
                            timeToggles.getY() - btnHeight,
                            comboWidth,
                            btnHeight);
    
    gonioControl.setBounds(stereoMeterPeak.getX() - (padding * 2) - comboWidth,
                           padding,
                           comboWidth,
                           120);
    
    viewToggles.setBounds(stereoMeterPeak.getX() - (padding * 2) - comboWidth,
                           stereoImageMeter.getBottom() - 135,
                           comboWidth,
                           135);
    
#if defined(GAIN_TEST_ACTIVE)
    gainSlider.setBounds(stereoMeterRms.getRight(), padding * 2, 20, 320);
#endif
}

void PFMProject10AudioProcessorEditor::timerCallback()
{
    if ( audioProcessor.fifo.getNumAvailable() > 0 )
    {
        while ( audioProcessor.fifo.pull(incomingBuffer) )
        {
            // do nothing else - just looping through until incomingBuffer = most recent available buffer
        }
        
        auto numSamples = incomingBuffer.getNumSamples();
                
        auto rmsL = incomingBuffer.getRMSLevel(0, 0, numSamples);
        auto rmsR = incomingBuffer.getRMSLevel(1, 0, numSamples);
        auto rmsDbL = juce::Decibels::gainToDecibels(rmsL, Globals::negInf());
        auto rmsDbR = juce::Decibels::gainToDecibels(rmsR, Globals::negInf());
        stereoMeterRms.update(rmsDbL, rmsDbR);
        
        auto peakL = incomingBuffer.getMagnitude(0, 0, numSamples);
        auto peakR = incomingBuffer.getMagnitude(1, 0, numSamples);
        auto peakDbL = juce::Decibels::gainToDecibels(peakL, Globals::negInf());
        auto peakDbR = juce::Decibels::gainToDecibels(peakR, Globals::negInf());
        stereoMeterPeak.update(peakDbL, peakDbR);
        
        histograms.update(HistogramTypes::RMS, rmsDbL, rmsDbR);
        histograms.update(HistogramTypes::PEAK, peakDbL, peakDbR);
        
        stereoImageMeter.update(incomingBuffer);
    }
}

void PFMProject10AudioProcessorEditor::initToggleGroupCallbacks(const ToggleGroup& toggleGroup, const std::vector<CustomToggle*>& togglePtrs)
{
    for ( size_t i = 0; i < togglePtrs.size(); ++i )
    {
        togglePtrs[i]->onClick = [i, this, toggleGroup] { updateParams(toggleGroup, i+1); };
    }
}

void PFMProject10AudioProcessorEditor::updateParams(const ToggleGroup& toggleGroup, const int& selectedId)
{
    switch (toggleGroup)
    {
        case ToggleGroup::DecayRate:
        {
            stereoMeterRms.setDecayRate(selectedId);
            stereoMeterPeak.setDecayRate(selectedId);
            timeToggles.decayRate.setSelectedValue(selectedId);
            break;
        }
        case ToggleGroup::AverageTime:
        {
            stereoMeterRms.resizeAverager(selectedId);
            stereoMeterPeak.resizeAverager(selectedId);
            timeToggles.avgDuration.setSelectedValue(selectedId);
            break;
        }
        case ToggleGroup::MeterView:
        {
            stereoMeterRms.setMeterView(selectedId);
            stereoMeterPeak.setMeterView(selectedId);
            viewToggles.meterView.setSelectedValue(selectedId);
            break;
        }
        case ToggleGroup::HoldTime:
        {
            stereoMeterRms.setTickHoldTime(selectedId);
            stereoMeterPeak.setTickHoldTime(selectedId);
            timeToggles.holdTime.setSelectedValue(selectedId);
            holdResetBtns.resetButton.setVisible( (selectedId == 6 && holdResetBtns.holdButton.getToggleState()) );
            break;
        }
        case ToggleGroup::HistView:
        {
            histograms.setView(selectedId);
            viewToggles.histView.setSelectedValue(selectedId);
            break;
        }
    }
}
