/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>

#define GAIN_TEST_ACTIVE

//==============================================================================

template<typename T>
struct Fifo
{
    void prepare(int numSamples, int numChannels)
    {
        for ( auto& buffer : buffers)
        {
            buffer.setSize(numChannels, numSamples, false, true, true);
            buffer.clear();
        }
    }
    
    bool push(const T& t)
    {
        auto write = fifo.write(1);
        if ( write.blockSize1 > 0 )
        {
            buffers[write.startIndex1] = t;
            return true;
        }
        return false;
    }
    
    bool pull(T& t)
    {
        auto read = fifo.read(1);
        if ( read.blockSize1 > 0 )
        {
            t = buffers[read.startIndex1];
            return true;
        }
        return false;
    }
    
    int getNumAvailable() const
    {
        return fifo.getNumReady();
    }
    
private:
    static constexpr int Capacity = 5;
    std::array<T, Capacity> buffers;
    juce::AbstractFifo fifo{Capacity};
};

//==============================================================================
/**
*/
class PFMProject10AudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PFMProject10AudioProcessor();
    ~PFMProject10AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    Fifo<juce::AudioBuffer<float>> fifo;
    
#if defined(GAIN_TEST_ACTIVE)
    static juce::AudioProcessorValueTreeState::ParameterLayout getParameterLayout();
    
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters", getParameterLayout() };
    
    juce::dsp::Oscillator<float> sineOsc;

    juce::dsp::Gain<float> gain;
    juce::AudioParameterFloat* gainParam { nullptr };
#endif

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PFMProject10AudioProcessor)
};
