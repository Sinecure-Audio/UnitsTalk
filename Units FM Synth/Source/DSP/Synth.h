#pragma once

#include "SmoothedValueBuffer.h"
#include "Oscillator.h"
#include "DSPUnitUtilities.h"
#include "../../../Units/include/Units.h"
#include <chrono>

//==============================================================================
// A descriptor for a sound that JUCE's MIDI synth can play
// This one tells the synth our sound can be played on any note or channel
class UniversalSound : public SynthesiserSound
{
public:
	UniversalSound() {}

	bool appliesToNote(int /*midiNoteNumber*/) override { return true; }
	bool appliesToChannel(int /*midiChannel*/) override { return true; }
};

//==============================================================================
//A synthesizer voice that plays an FMPair of sine waves enveloped by an ADSR
class MIDIFMVoice : public SynthesiserVoice
{
public:
	MIDIFMVoice(const SmoothedValueBuffer<float>& depthBuffer,
                  const SmoothedValueBuffer<float>& ratioBuffer)
    : modulationDepthBuffer(depthBuffer), modulationRatioBuffer(ratioBuffer) {}

	bool canPlaySound(SynthesiserSound* sound) override {
		return dynamic_cast<UniversalSound*> (sound) != nullptr;
	}

    //initialise the oscillator and envelope
	void startNote(int midiNoteNumber,
                   float velocity,
                   SynthesiserSound* /*sound*/,
                   int /*currentPitchWheelPosition*/) override {
		const auto sampleRate = static_cast<float>(getSampleRate());

		envelope.noteOn();
		velocityLevel = velocity;
        
		frequency = static_cast<float>(MidiMessage::getMidiNoteInHertz(midiNoteNumber));
        fmOsc.prepare(sampleRate, 32);
        fmOsc.setFrequency(frequency);
	}

    //begin note off if we are allowed to fade out, immediatly stop the note if we aren't
	void stopNote(float /*velocity*/, bool allowTailOff) override {
		if (allowTailOff)
			envelope.noteOff();
		else
			clearCurrentNote();
	}

	void pitchWheelMoved(int /*newValue*/) override {}

	void controllerMoved(int /*controllerNumber*/, int /*newValue*/) override {}

	void setEnvelopeParameters(TimedADSRParameters* newEnvelopeParameters) {
		envelopeParameters = newEnvelopeParameters;
	}

    //update the envelope parameters and perform the samples in the buffer
    void renderNextBlock(AudioBuffer<double>& outputBuffer,
                         int startSample,
                         int numSamples) override
    {
        envelope.setSampleRate(getSampleRate());
        envelope.setParameters(*envelopeParameters);

        for (int currentSample = startSample; currentSample < startSample+numSamples; ++currentSample)
            outputBuffer.addSample(0, currentSample, performSample(static_cast<size_t>(currentSample)));
        
    }
    
	void renderNextBlock(AudioBuffer<float>& outputBuffer,
                         int startSample,
                         int numSamples) override
    {
		envelope.setSampleRate(getSampleRate());
		envelope.setParameters(*envelopeParameters);

        for (int currentSample = startSample; currentSample < startSample+numSamples; ++currentSample)
            outputBuffer.addSample(0, currentSample, static_cast<float>(performSample(static_cast<size_t>(currentSample))));
	}

private:
    //update the oscillator's depth and ratio for this sample
    //Then multiply it by the envelope and scaled velocity
    double performSample(const size_t& currentSample) {
        fmOsc.setFMDepth(modulationDepthBuffer[currentSample]);
        fmOsc.setFMRatio(modulationRatioBuffer[currentSample]);
        return fmOsc.perform() * envelope.getNextSample()* std::sqrt(velocityLevel)*.1;
    }
    
    FMPair<SinOscillator<double>> fmOsc;

	ADSR envelope;
	TimedADSRParameters* envelopeParameters;
    
    float velocityLevel{0}, frequency{0};
    const SmoothedValueBuffer<float>& modulationDepthBuffer;
    const SmoothedValueBuffer<float>& modulationRatioBuffer;
};

// A synthesizer voice that plays an FMPair of sinewaves
// that are then run through a lowpass filter.
// A release envelope is applied when a note off or pressure of 0 is received.
// The FM oscillator's pitch, volume, and modulation depth are modulated
// by the controller's pitch, pressure, and brightness, respectively.
// The cutoff filter is also modulated by the controller's pressure.
class MPEFMVoice : public MPESynthesiserVoice
{
public:
	//==============================================================================
	MPEFMVoice(const SmoothedValueBuffer<float>& cutoffBuffer,
                     const SmoothedValueBuffer<float>& resonanceBuffer,
                     const SmoothedValueBuffer<float>& newModulationDepthBuffer,
                     const SmoothedValueBuffer<float>& ratioBuffer,
                     const SmoothedValueBuffer<float>& pressureBuffer,
                     const SmoothedValueBuffer<float>& timbreBuffer,
                     const Decibel<float>& voiceGain,
                     const std::chrono::duration<float>& release)
    : cutoffFrequencyBuffer(cutoffBuffer),
      resonanceAmountBuffer(resonanceBuffer),
      modulationDepthBuffer(newModulationDepthBuffer),
      modulationRatioBuffer(ratioBuffer),
      pressureCoefficientBuffer(pressureBuffer),
      slideCoefficientBuffer(timbreBuffer),
      decibelGain(voiceGain),
      releaseTime(release)
    {}

	//==============================================================================
	void noteStarted() override
	{
		jassert(currentlyPlayingNote.isValid());
		jassert(currentlyPlayingNote.keyState == MPENote::keyDown
			|| currentlyPlayingNote.keyState == MPENote::keyDownAndSustained);

		// get data from the current MPENote
        pressure.reset(currentSampleRate, smoothingLengthInSeconds.count());
        const auto pressureTarget = std::max(getCurrentlyPlayingNote().noteOnVelocity.asUnsignedFloat(),
                                             currentlyPlayingNote.pressure.asUnsignedFloat());
		pressure.setCurrentAndTargetValue(pressureTarget);
		frequency.setCurrentAndTargetValue(static_cast<float>(currentlyPlayingNote.getFrequencyInHertz()));
        
        //Initialize timbre smoother
        timbre = SmoothedValue<float>(0.0f);
		timbre.setCurrentAndTargetValue(std::sqrt(static_cast<float>(currentlyPlayingNote.timbre.asUnsignedFloat())));
        
        // turn off the flag that initialise the release
        released = false;
        // reset the oscillator phase
        fmOsc.reset();
        // and update the voice gain
        amplitudeGain = decibelGain;
	}

    //if note is stopped, find an appropriate pressure value
    //and smooth towards it over the release time
	void noteStopped(bool allowTailOff) override {
        if (allowTailOff) {
            if(!released) {
                const auto nextPressure = pressure.getNextValue();
                pressure.reset(currentSampleRate, releaseTime.count());
            
                pressure.setCurrentAndTargetValue(std::max(nextPressure, previousPressure));
                pressure.setTargetValue(std::numeric_limits<float>::min());
                released = true;
            }
        }
        else
            clearCurrentNote();
    }

    // update the note pressure, if the note is receiving pressure and not released.
    // If pressure is 0 in a note that has received pressure, that means that the note has been released
    // So we trigger a note off
	void notePressureChanged() override {
        if(!released) {
            if(currentlyPlayingNote.pressure.asUnsignedFloat() >= 1.0f/pow(2.0f, 14.0f)){
                previousPressure = pressure.getTargetValue();
                pressure.setTargetValue(currentlyPlayingNote.pressure.asUnsignedFloat());
            }
            //if current and previous pressure are 0, and we're still playing, then we probably never had pressure to begin with
            // buf if it wasn't 0, let's stop
            else if(previousPressure != 0.0f)
                noteStopped(true);
        }
	}

	void notePitchbendChanged() override {
        frequency.setTargetValue(static_cast<float>(currentlyPlayingNote.getFrequencyInHertz()));
	}

	void noteTimbreChanged() override {
        timbre.setTargetValue(std::sqrt(static_cast<float>(currentlyPlayingNote.timbre.asUnsignedFloat())));
	}

	void noteKeyStateChanged() override {}

    // update sampling rate, stop the note if it's playing, and reset update the smoothing rate of all of the smoothers
	void setCurrentSampleRate(double newRate) override {
		if (currentSampleRate != newRate) {
			noteStopped(false);
			currentSampleRate = newRate;
            timbre.reset(currentSampleRate,    smoothingLengthInSeconds.count());
            pressure.reset(currentSampleRate,  smoothingLengthInSeconds.count());
            frequency.reset(currentSampleRate, smoothingLengthInSeconds.count());
		}
	}

    //prepare and reset the oscillator and filter
	void prepare(const juce::dsp::ProcessSpec& spec)
	{
		currentSampleRate = spec.sampleRate;
        
        fmOsc.prepare(spec.sampleRate, static_cast<int>(spec.maximumBlockSize));
        fmOsc.setFrequency(frequency.getTargetValue());
        fmOsc.reset();

        filter.prepare(spec);
        filter.reset();
	}

	//==============================================================================
    
	void renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override {
        processBlock(outputBuffer, startSample, numSamples);
	}
    
    void renderNextBlock(AudioBuffer<double>& outputBuffer, int startSample, int numSamples) override {
        processBlock(outputBuffer, startSample, numSamples);
    }
    
    //generate a block of samples from the synthesizer
    template<typename SampleType>
    void processBlock(AudioBuffer<SampleType>& outputBuffer, int startSample, int numSamples) {
        for (size_t i = static_cast<size_t>(startSample);
                    i < static_cast<size_t>(startSample + numSamples); ++i) {
            //Update all of the mpe-control smoothers
            const auto noteFrequency = frequency.getNextValue();
            const auto timbreValue = timbre.getNextValue()*slideCoefficientBuffer[i];
            const auto amplitudeMultiplier = pow(pressure.getNextValue(), 2.0f)*pressureCoefficientBuffer[i];
            
            //calculate the cutoff frequency for this sample and set the filter cutoff.
            const auto baseFrequency = std::min(noteFrequency, cutoffFrequencyBuffer[i]);
            const auto highFrequency = std::max(noteFrequency, cutoffFrequencyBuffer[i]);
            const auto filterPressureOffset =  baseFrequency+(highFrequency-baseFrequency)*amplitudeMultiplier;
            
            filter.parameters->setCutOffFrequency(currentSampleRate,
                                                        filterPressureOffset, QCoefficient<float>{ResonanceCoefficient<float>{resonanceAmountBuffer[i]}}.count());
            
            // update the oscillator parameters and perform the oscillator.
            const auto effectiveModDepth = modulationDepthBuffer[i]*(timbreValue+.5f);
            fmOsc.setFrequency(noteFrequency);
            fmOsc.setFMDepth(effectiveModDepth);
            fmOsc.setFMRatio(modulationRatioBuffer[static_cast<size_t>(i)]);
            const auto oscillatorOutput = fmOsc.perform();
            
            // add the scaled output of the filter to the current sample
            *outputBuffer.getWritePointer(0, static_cast<int>(i))
                //cast the result to the sample type
                += static_cast<SampleType>(filter.processSample(oscillatorOutput)
                                            * .1f
                                            * amplitudeGain
                                            * amplitudeMultiplier);

            //if the release segment is done, clear the current note.
            if(released && !pressure.isSmoothing())
                clearCurrentNote();
        }
    }

private:
	//==============================================================================
	SmoothedValue<float> timbre, frequency, pressure;
    
    FMPair<SinOscillator<double>> fmOsc;

    float previousPressure{0};
    bool released{false};

    dsp::StateVariableFilter::Filter<double> filter;

    const SmoothedValueBuffer<float>& cutoffFrequencyBuffer;
    const SmoothedValueBuffer<float>& resonanceAmountBuffer;
    const SmoothedValueBuffer<float>& modulationDepthBuffer;
    const SmoothedValueBuffer<float>& modulationRatioBuffer;
    const SmoothedValueBuffer<float>& pressureCoefficientBuffer;
    const SmoothedValueBuffer<float>& slideCoefficientBuffer;
    
    const Decibel<float>& decibelGain;
    const std::chrono::duration<float>& releaseTime;
    
    Amplitude<float> amplitudeGain{0.0};

	static constexpr std::chrono::duration<double> smoothingLengthInSeconds = std::chrono::milliseconds{50};
};

class AudioEngine : public juce::MPESynthesiser
{
public:
	//==============================================================================
	AudioEngine()
	{
		setVoiceStealingEnabled(true);
	}

	//==============================================================================
	void prepare(const juce::dsp::ProcessSpec& spec) noexcept
	{
		setCurrentPlaybackSampleRate(spec.sampleRate);

		for (auto* v : voices)
			dynamic_cast<MPEFMVoice*> (v)->prepare(spec);
	}

private:
	//==============================================================================
	void renderNextSubBlock(AudioBuffer<float>& outputAudio, int startSample, int numSamples) override
	{
		MPESynthesiser::renderNextSubBlock(outputAudio, startSample, numSamples);
	}
};
