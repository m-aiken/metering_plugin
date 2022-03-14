/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PFMProject10AudioProcessor::PFMProject10AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // default values for value tree
    valueTree.setProperty("DecayTime",          3, nullptr); // -12dB/s
    valueTree.setProperty("AverageTime",        3, nullptr); // 500ms
    valueTree.setProperty("MeterViewMode",      1, nullptr); // Both
    valueTree.setProperty("GoniometerScale",  100, nullptr);
    valueTree.setProperty("EnableHold",      true, nullptr);
    valueTree.setProperty("HistogramView",      1, nullptr); // Stacked
    valueTree.setProperty("PeakThreshold",      1, nullptr);
    valueTree.setProperty("RMSThreshold",       1, nullptr);
    
#if defined(GAIN_TEST_ACTIVE)
    gainParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Gain"));
    jassert(gainParam != nullptr);
#endif
}

PFMProject10AudioProcessor::~PFMProject10AudioProcessor()
{
}

//==============================================================================
const juce::String PFMProject10AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PFMProject10AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PFMProject10AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PFMProject10AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PFMProject10AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PFMProject10AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PFMProject10AudioProcessor::getCurrentProgram()
{
    return 0;
}

void PFMProject10AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PFMProject10AudioProcessor::getProgramName (int index)
{
    return {};
}

void PFMProject10AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PFMProject10AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    fifo.prepare(samplesPerBlock, getTotalNumOutputChannels());
    
#if defined(GAIN_TEST_ACTIVE)
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    
    sineOsc.prepare(spec);
    sineOsc.initialise([](float f) { return std::sin(f); });
    sineOsc.setFrequency(440);
    
    gain.prepare(spec);
    gain.setRampDurationSeconds(0.05);
#endif
}

void PFMProject10AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PFMProject10AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PFMProject10AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

#if defined(GAIN_TEST_ACTIVE)
    for ( int sampleIdx = 0; sampleIdx < buffer.getNumSamples(); ++sampleIdx )
    {
        auto newVal = sineOsc.processSample(buffer.getSample(0, sampleIdx));
        
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            buffer.setSample(channel, sampleIdx, newVal);
        }
        
        // out of phase test
//        buffer.setSample(1, sampleIdx, newVal * -1.f);
    }
    
    gain.setGainDecibels(gainParam->get());
    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    gain.process(context);
#endif
    
    fifo.push(buffer);
}

//==============================================================================
bool PFMProject10AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PFMProject10AudioProcessor::createEditor()
{
    return new PFMProject10AudioProcessorEditor (*this);
}

//==============================================================================
void PFMProject10AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::MemoryOutputStream outputStream(destData, true);
    valueTree.writeToStream(outputStream);
}

void PFMProject10AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    
    if
    (
        tree.isValid() &&
        tree.hasProperty("DecayTime") &&
        tree.hasProperty("AverageTime") &&
        tree.hasProperty("MeterViewMode") &&
        tree.hasProperty("GoniometerScale") &&
        tree.hasProperty("EnableHold") &&
        tree.hasProperty("HistogramView") &&
        tree.hasProperty("PeakThreshold") &&
        tree.hasProperty("RMSThreshold")
    )
    {
        valueTree = tree;
    }
}
#if defined(GAIN_TEST_ACTIVE)
juce::AudioProcessorValueTreeState::ParameterLayout PFMProject10AudioProcessor::getParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("Gain",
                                                           "Gain",
                                                           juce::NormalisableRange<float>(NegativeInfinity, MaxDecibels, 1.f, 1.f),
                                                           0.f));
    
    return layout;
}
#endif
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PFMProject10AudioProcessor();
}
