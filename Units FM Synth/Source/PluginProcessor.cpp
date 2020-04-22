/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <chrono>

//==============================================================================
UnitsFmSynthAudioProcessor::UnitsFmSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ),
    #endif
        parameterTree (*this, &undo, "state", [&](){
            auto arr = createParameterLayout();
            return AudioProcessorValueTreeState::ParameterLayout{arr.begin(), arr.end()};}())

{
        if(!parameterTree.state.getChildWithName("Properties").isValid())
                parameterTree.state.appendChild({"Properties",
                                     {{"MIDIMode", 0},
                                      { "width",  900 }, { "height", 460 }},
                                      {}}, nullptr);

            initialiseSynth();
            
            ResonanceCoefficient<float> res = *static_cast<ResonanceParameter*>(parameterTree.getParameter("resonance"));
            res /= 100.05f;
            midiSynthFilterParams->setCutOffFrequency(performRate,
                 *parameterTree.getRawParameterValue("cutoff"), res);

            mpeSynth.setZoneLayout([]{MPEZoneLayout layout; layout.setLowerZone(15, 48); layout.setUpperZone(0); return layout;}());
            mpeSynth.setVoiceStealingEnabled(true);
            
            auto propertyTree = parameterTree.state.getChildWithName ("Properties");
            if(propertyTree.isValid()) {
                cachedMode.referTo(propertyTree, "MIDIMode",  nullptr);
                if (cachedMode.get() == 0)
                    synthMode.store(ControlDisplayMode::AutoMIDI);
                else if (cachedMode.get() == 1)
                    synthMode.store(ControlDisplayMode::AutoMPE);
                else if (cachedMode.get() == 2)
                    synthMode.store(ControlDisplayMode::MIDI);
                else if (cachedMode.get() == 3)
                    synthMode.store(ControlDisplayMode::MPE);
            }
            
            previousMidiMode = synthMode;
            
            cutoffParamBuffer.setSmoothingTime(std::chrono::duration<float, std::milli>{20.0f});
            resonanceParamBuffer.setSmoothingTime(std::chrono::duration<float, std::milli>{20.0f});
            modDepthParamBuffer.setSmoothingTime(std::chrono::duration<float, std::milli>{20.0f});
            modRatioParamBuffer.setSmoothingTime(std::chrono::duration<float, std::milli>{20.0f});
            pressureParamBuffer.setSmoothingTime(std::chrono::duration<float, std::milli>{20.0f});
            slideParamBuffer.setSmoothingTime(std::chrono::duration<float, std::milli>{20.0f});
            mpeGainParamBuffer.setSmoothingTime(std::chrono::duration<float, std::milli>{20.0f});
            
            reset();
}


//==============================================================================
const String UnitsFmSynthAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool UnitsFmSynthAudioProcessor::acceptsMidi() const {
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool UnitsFmSynthAudioProcessor::producesMidi() const {
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool UnitsFmSynthAudioProcessor::isMidiEffect() const {
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double UnitsFmSynthAudioProcessor::getTailLengthSeconds() const {return 0.0;}

int UnitsFmSynthAudioProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int UnitsFmSynthAudioProcessor::getCurrentProgram() {return 0;}

//==============================================================================
void UnitsFmSynthAudioProcessor::prepareToPlay (double newSampleRate,
                                                int samplesPerBlock) {
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    midiSynth.setCurrentPlaybackSampleRate (newSampleRate);

    mpeSynth.prepare({newSampleRate, uint32(samplesPerBlock), 2});
    
    floatBuffer.setSize(std::max(getTotalNumInputChannels(),
                                 getTotalNumOutputChannels()),
                        samplesPerBlock);

#if JUCE_STANDALONE_APPLICATION // || JucePlugin_Build_Unity || JucePlugin_Build_IAA
    if(wrapperType == AudioProcessor::WrapperType::wrapperType_Standalone) {
        keyboardState.reset();
        midiMessageCollector.reset(performRate);
    }
#endif
    
    midiSeparator.prepare(newSampleRate, samplesPerBlock);

    performRate = newSampleRate;
    
    ResonanceCoefficient<float> res = *static_cast<ResonanceParameter*>(parameterTree.getParameter("resonance"));
    res /= 100.05f;
    
    midiSynthFilterParams->setCutOffFrequency(performRate,
         *parameterTree.getRawParameterValue("cutoff"), res);

    cutoffParamBuffer.prepare(performRate, samplesPerBlock);
    resonanceParamBuffer.prepare(performRate, samplesPerBlock);
    modDepthParamBuffer.prepare(performRate, samplesPerBlock);
    modRatioParamBuffer.prepare(performRate, samplesPerBlock);
    pressureParamBuffer.prepare(performRate, samplesPerBlock);
    slideParamBuffer.prepare(performRate, samplesPerBlock);
    mpeGainParamBuffer.prepare(performRate, samplesPerBlock);

    reset();
}

void UnitsFmSynthAudioProcessor::reset() {
#if JUCE_STANDALONE_APPLICATION
    if(wrapperType == AudioProcessor::WrapperType::wrapperType_Standalone)
        keyboardState.reset();
#endif
    midiSynthFilter.reset();
    
    cutoffParamBuffer.reset();
    resonanceParamBuffer.reset();
    modDepthParamBuffer.reset();
    modRatioParamBuffer.reset();
    pressureParamBuffer.reset();
    slideParamBuffer.reset();
    mpeGainParamBuffer.reset();
    
    midiSeparator.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool UnitsFmSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const {
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

//Process buffer.
 void UnitsFmSynthAudioProcessor::processBlock (AudioBuffer<float>& buffer,
                                                MidiBuffer& midiMessages) {
     jassert (!this->isUsingDoublePrecision());
	 this->process (buffer, midiMessages);
 }

//convert double buffer to float buffer, and then process it
 void UnitsFmSynthAudioProcessor::processBlock (AudioBuffer<double>& buffer,
                                                MidiBuffer& midiMessages) {
	 jassert(this->isUsingDoublePrecision());
     floatBuffer.makeCopyOf(buffer, true);
     this->process (floatBuffer, midiMessages);
     buffer.makeCopyOf(floatBuffer, true);
 }

//==============================================================================
bool UnitsFmSynthAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* UnitsFmSynthAudioProcessor::createEditor() {
    return new UnitsFmSynthAudioProcessorEditor (*this, std::ref(synthMode));
}

//==============================================================================
void UnitsFmSynthAudioProcessor::getStateInformation (MemoryBlock& destData) {
	// Store an xml representation of our state.
	std::unique_ptr<XmlElement> xmlState(parameterTree.copyState().createXml());

	if (xmlState.get() != nullptr)
		copyXmlToBinary(*xmlState, destData);
}

void UnitsFmSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {
	// Restore our plug-in's state from the xml representation stored in the above
	// method.
	std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

	if (xmlState.get() != nullptr)
		parameterTree.replaceState(ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new UnitsFmSynthAudioProcessor();
}
