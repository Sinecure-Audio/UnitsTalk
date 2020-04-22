/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DSP/DSPUnitUtilities.h"
#include "DSP/Synth.h"
#include "../../Examples/AudioParameterUnit.h"
#include "Utilities/MidiMPESeparator.h"

#include <chrono>

using ResonanceParameter = AudioParameterUnit<ResonanceCoefficient<float>, float>;
using SecondsParameter = AudioParameterUnit<std::chrono::duration<float>, float>;
using DecibelParameter = AudioParameterUnit<Decibel<float>, float>;

enum class ControlDisplayMode {
    AutoMIDI,
    AutoMPE,
    MIDI,
    MPE
};

//==============================================================================
/**
*/
class UnitsFmSynthAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    UnitsFmSynthAudioProcessor();
    ~UnitsFmSynthAudioProcessor(){};

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    
    void reset() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;
    void processBlock (AudioBuffer<double>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int /*index*/) override {}
    const String getProgramName (int /*index*/) override {return{};}
    void changeProgramName (int /*index*/, const String& /*newName*/) override {}

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	auto& getParameterTree() {return parameterTree;}
    auto& getUndoManager() {return undo;}

#if JUCE_STANDALONE_APPLICATION // || JucePlugin_Build_Unity || JucePlugin_Build_IAA
	auto& getMIDIKeyboardState() {return keyboardState;}
#endif
    
    //create parameter groupings and their names, ids, and default values.
    auto createParameterLayout() const {
        std::array<std::unique_ptr<AudioProcessorParameterGroup>, 4> paramLayout {
            std::make_unique<AudioProcessorParameterGroup>("fmControls", "FM Controls", " ",
                 std::make_unique<AudioParameterFloat>("modDepth", "Modulation Depth", NormalisableRange<float>(0.0f, 100.0f, 0.001f), 0.0f),
                 std::make_unique<AudioParameterFloat>("modRatio", "FM Ratio", NormalisableRange<float>(0.0f, 1600.0f, 0.001f, .5f), 100.0f)),
            
            std::make_unique<AudioProcessorParameterGroup>("filterControls", "Filter Controls", " ",
                 std::make_unique<AudioParameterFloat>("cutoff", "Filter Cutoff Frequency", NormalisableRange<float>(5.0f, 20000.0f, .001f, 1.0f/3.2f), 800.0f),
                 std::make_unique<ResonanceParameter>("resonance", "Filter Resonance", NormalisableRange<float>(.025f, 100.0f, .001f), .025f)),

             std::make_unique<AudioProcessorParameterGroup>("midiEnvelopeControls", "MIDI Envelope Controls", " ",
                 std::make_unique<SecondsParameter>("attack",  "Attack",  NormalisableRange<float>(0.0f, 10.0f, .001f, 1.0f/3.2f), .02f),
                 std::make_unique<SecondsParameter>("decay",   "Decay",   NormalisableRange<float>(0.0f, 10.0f, .001f, 1.0f / 3.2f), 0.0f),
                 std::make_unique<DecibelParameter>("sustain", "Sustain", NormalisableRange<float>(-120.0f,  0.0f, .001f),  0.0f),
                 std::make_unique<SecondsParameter>("release", "Release", NormalisableRange<float>(0.0f, 10.0f, .001f, 1.0f / 3.2f), .02f)),
        
             std::make_unique<AudioProcessorParameterGroup>("mpeEnvelopeControls", "MPE Envelope Controls", " ",
                 std::make_unique<SecondsParameter>("pressureCF", "Pressure Amount",   NormalisableRange<float>(0.0f, 100.0f, .001f), 100.0f),
                 std::make_unique<SecondsParameter>("slideCF", "Slide Amount",   NormalisableRange<float>(0.0f, 100.0f, .001f), 100.0f),
                 std::make_unique<DecibelParameter>("voiceGain", "Voice Gain", NormalisableRange<float>(-120.0f,  0.0f, .001f),  0.0f),
                 std::make_unique<SecondsParameter>("mpeRelease", "Release", NormalisableRange<float>(0.0f, 10.0f, .001f, 1.0f / 3.2f), .02f))
           };
        return paramLayout;
    }

private:
    //actually generate the sound
    template <typename FloatType>
    void process (AudioBuffer<FloatType>& buffer, MidiBuffer& midiMessages) {
		ScopedNoDenormals noDenorms{};

        envelopeParameters.attack =  *static_cast<SecondsParameter*>(parameterTree.getParameter("attack"));
        envelopeParameters.decay =
            *static_cast<SecondsParameter*>(parameterTree.getParameter("decay"));
        envelopeParameters.sustain = Decibel<float>{*static_cast<DecibelParameter*>(parameterTree.getParameter("sustain"))};
        envelopeParameters.release = *static_cast<SecondsParameter*>(parameterTree.getParameter("release"));

		cutoffParameterValue = *parameterTree.getRawParameterValue("cutoff");
        resonanceParameterValue = *static_cast<ResonanceParameter*>(parameterTree
                                                                    .getParameter("resonance"));
        // a value of 100 will cause the filter will blow up
        // so divide by something a bit smaller to keep it in bounds.
        resonanceParameterValue /= 100.05f;
        
        const auto numSamples = buffer.getNumSamples();
        
        //Clear channel 0 as all synth operations add onto what's in there, and there's no guarantee it's empty.
        for(int i = 0; i < buffer.getNumChannels(); ++i)
            buffer.clear(i, 0, numSamples);
        
        //fill all of the smoother buffers with this block's param trajectories.
        cutoffParamBuffer.fillBuffer(*parameterTree.getRawParameterValue("cutoff"));
        //use a float buffer for resonance and dB since juce's Smoother won't work on Unit Types. Grr.....
        resonanceParamBuffer.fillBuffer(resonanceParameterValue.count());
        modDepthParamBuffer .fillBuffer(*parameterTree.getRawParameterValue("modDepth")/100.0f);
        modRatioParamBuffer .fillBuffer(*parameterTree.getRawParameterValue("modRatio")/100.0f);
        mpeGainParamBuffer.fillBuffer(Decibel<float>{
                                        *static_cast<DecibelParameter*>(parameterTree
                                                                        .getParameter("voiceGain"))}
                                                                        .count());
        pressureParamBuffer.fillBuffer(*parameterTree.getRawParameterValue("pressureCF")/100.0f);
        slideParamBuffer.fillBuffer(*parameterTree.getRawParameterValue("slideCF")/100.0f);

#if JUCE_STANDALONE_APPLICATION // || JucePlugin_Build_Unity || JucePlugin_Build_IAA
        // Now pass any incoming midi messages to our keyboard state object, and let it
        // add messages to the buffer if the user is clicking on the on-screen keys
        if(wrapperType == AudioProcessor::WrapperType::wrapperType_Standalone) {
                midiMessageCollector.removeNextBlockOfMessages(midiMessages, buffer.getNumSamples());
                keyboardState.processNextMidiBuffer (midiMessages, 0, numSamples, true);
        }
#endif
        
        // if the selected synth mode is auto, sort the incoming midi messages into midi and mpe
        // messages buffers, and play both synths. Otherwise, play the selected synth.
        if(synthMode == ControlDisplayMode::AutoMIDI || synthMode == ControlDisplayMode::AutoMPE) {
            const auto& [midiBuffer, mpeBuffer] = midiSeparator.separateBuffers(midiMessages);
        
            if(!midiBuffer.isEmpty() && !midiReceived.load())
                midiReceived.store(true);
            if(!mpeBuffer.isEmpty() && !mpeReceived.load())
                mpeReceived.store(true);
        
            //if both buffers have data, don't change the active component.
            if(!midiBuffer.isEmpty() ^ !mpeBuffer.isEmpty() && (synthMode == ControlDisplayMode::AutoMIDI || synthMode == ControlDisplayMode::AutoMPE))
                synthMode.store(midiBuffer.isEmpty() ? ControlDisplayMode::AutoMPE : ControlDisplayMode::AutoMIDI);
            
            midiSynth.renderNextBlock (buffer, midiBuffer, 0, numSamples);
            //Apply the lowpass filter to the midi synth output in the buffer
            filterBuffer(buffer);
            
            //run the mpeSynth after the filters because the mpeSynth filters internally.
            mpeSynth.renderNextBlock(buffer, mpeBuffer, 0, numSamples);
        }
        
        // and now get our synth to process these midi events and generate its output.
        else if(synthMode == ControlDisplayMode::MIDI) {
            midiSynth.renderNextBlock (buffer, midiMessages, 0, numSamples);
            //Apply the lowpass filter to the midi synth output in the buffer
            filterBuffer(buffer);
            if(!midiMessages.isEmpty())
                midiReceived.store(true);
        }
        else if(synthMode == ControlDisplayMode::MPE) {
            mpeSynth.renderNextBlock(buffer, midiMessages, 0, numSamples);
            if(!midiMessages.isEmpty())
                mpeReceived.store(true);
        }

        //copy the channel we just generated to all other channels
        for(int i = 1; i < buffer.getNumChannels(); ++i)
            buffer.copyFrom (i, 0, buffer.getReadPointer(0), numSamples);
    }

    // this is kept up to date with the midi messages that arrive
    // so the midi keyboard component can display the active midi notes
#if JUCE_STANDALONE_APPLICATION//  || JucePlugin_Build_Unity || JucePlugin_Build_IAA
	MidiKeyboardState keyboardState{};
	MidiMessageCollector midiMessageCollector;
#endif

    // Our plug-in's current state
    AudioProcessorValueTreeState parameterTree;
	UndoManager undo;

    //stored perform rate for the benefit of the midi synth filters
    double performRate{44100.0};
    
    //A buffer for converting a double buffer to floating point
    AudioBuffer<float> floatBuffer;
    
    
    //Buffers for holding smoothed parameter values.
    //These allow the synths to access the correct param value for each sample
    SmoothedValueBuffer<float> cutoffParamBuffer{},   resonanceParamBuffer{}, modDepthParamBuffer{},
        modRatioParamBuffer{}, pressureParamBuffer{}, slideParamBuffer{},     mpeGainParamBuffer{};

    float modDepthParameterValue{ 0 }, cutoffParameterValue{20};
    
    ResonanceCoefficient<float> resonanceParameterValue{ .01f };

    std::atomic<bool> midiReceived = false, mpeReceived = false;
        
    //variables that store the current, previous, and cached (value tree) midi modes. Used to determine which synth is active, and to switch the editor's bottom display to the correct knobs.
    std::atomic<ControlDisplayMode> synthMode{ControlDisplayMode::AutoMIDI};
    ControlDisplayMode previousMidiMode{synthMode};
    CachedValue<int> cachedMode;

    static constexpr size_t numVoices = 10;

    //filters for the midi synth, both float and double.
        ReferenceCountedObjectPtr< SVFParams<double> > midiSynthFilterParams = ReferenceCountedObjectPtr< SVFParams<double> >(new SVFParams<double>());
    
    dsp::StateVariableFilter::Filter<double> midiSynthFilter = dsp::StateVariableFilter::Filter<double> (midiSynthFilterParams);
    
    TimedADSRParameters envelopeParameters;

    Synthesiser midiSynth;
    AudioEngine mpeSynth;
    
    //Seperator for splitting the input midiBuffer into a buffer for both MIDI and MPE
    MidiMpeSeperator midiSeparator{};

    void initialiseSynth() {
        // Add some voices...
        for (size_t i = 0; i < numVoices; ++i)
        {
            auto* voice = new MIDIFMVoice(modDepthParamBuffer, modRatioParamBuffer);
            voice->setEnvelopeParameters(&envelopeParameters);
            midiSynth.addVoice(voice);

            mpeSynth.addVoice(new MPEFMVoice(
                                       cutoffParamBuffer,
                                       resonanceParamBuffer,
                                       modDepthParamBuffer,
                                       modRatioParamBuffer,
                                       pressureParamBuffer,
                                       slideParamBuffer,
                                       *static_cast<DecibelParameter*>(parameterTree.getParameter("voiceGain")),
                                       *static_cast<SecondsParameter*>(parameterTree.getParameter("mpeRelease"))
                                       ));

        }

        // ..and give the midi synth a sound to play
        midiSynth.addSound (new UniversalSound());
    }
    
    // filter the buffer using the float or double filter, depending on the sample type
    template<typename FloatType>
    auto filterBuffer(AudioBuffer<FloatType>& buffer) {
        // filter the entire audio buffer with a lowpass svf filter
        for(size_t currentSampleIndex = 0; currentSampleIndex < static_cast<size_t>(buffer.getNumSamples()); ++currentSampleIndex) {
            auto& inputSample = buffer.getWritePointer(0, static_cast<int>(currentSampleIndex))[0];
            midiSynthFilterParams->setCutOffFrequency(performRate,
                     cutoffParamBuffer[currentSampleIndex],
                     ResonanceCoefficient<float>{resonanceParamBuffer[currentSampleIndex]});
            inputSample = static_cast<FloatType>(midiSynthFilter.processSample(inputSample));
        }
    }
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UnitsFmSynthAudioProcessor)
};
