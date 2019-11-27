using ResonanceParameter = AudioParameterUnit<ResonanceCoefficient<float>, float>;
using SecondsParameter = AudioParameterUnit<std::chrono::duration<float>, float>;
using DecibelParameter = AudioParameterUnit<Decibel<float>, float>;

class UnitsFilterAudioProcessor : public AudioProcessor
{
    UnitsFilterAudioProcessor() :             
    parameterTree (*this, &undo, "state",
           { // put some parameters in the tree, along with their ranges, scalings, and default values.
             std::make_unique<AudioParameterFloat>("cutoff", "Filter Cutoff Frequency", NormalisableRange<float>(5.0f, 20000.0f, .001f, 1.0f/3.2f), 800.0f),
             std::make_unique<ResonanceParameter>("resonance", "Filter Resonance", NormalisableRange<float>(0.0f, 100.0f, .001f), 0.01f),
             std::make_unique<AudioParameterFloat>("modDepth", "Modulation Depth", NormalisableRange<float>(0.0f, 100.0f, 0.001f), 0.0f),

             std::make_unique<SecondsParameter>("attack",  "Attack",  NormalisableRange<float>(0.0f, 10.0f, .001f, 1.0f/3.2f), .02f),
             std::make_unique<SecondsParameter>("decay",   "Decay",   NormalisableRange<float>(0.0f, 10.0f, .001f, 1.0f / 3.2f), 0.0f),
             std::make_unique<DecibelParameter>("sustain", "Sustain", NormalisableRange<float>(-120.0f,  0.0f, .01f),  0.0f),
             std::make_unique<SecondsParameter>("release", "Release", NormalisableRange<float>(0.0f, 10.0f, .001f, 1.0f / 3.2f), .02f)
           }
          )
    {}

//...

void updateParameters() {
        //get raw parameter values when the type in your DSP class is not a unit type
        midiSynthEnvelopeParameters.attack =  *parameterTree.getRawParameterValue("attack");
        midiSynthEnvelopeParameters.decay =   *parameterTree.getRawParameterValue("decay");
        midiSynthEnvelopeParameters.release = *parameterTree.getRawParameterValue("release");

        //You can construct a unit from a parameter of the same type as the unit
        const Decibel<float> sustainGain{*static_cast<DecibelParameter*>(parameterTree.getParameter("sustain"))};
        midiSynthEnvelopeParameters.sustain = Amplitude<float>{sustainGain}.count();

        //filter params, cutoff use float but resonance uses a unit type
        cutoffParameterValue = *parameterTree.getRawParameterValue("cutoff");
        resonanceParameterValue = *static_cast<ResonanceParameter*>(parameterTree.getParameter("resonance"));

        //mod depth is float
        modDepthParameterValue = *parameterTree.getRawParameterValue("modDepth");
}

//...

private:
    AudioParameterValueTreeState tree;

    //values for holding filter and fm parameters
    float modDepthParameterValue{ 0 }, cutoffParameterValue{20};
    ResonanceCoefficient<float> resonanceParameterValue{ .01f };

    //envelope parameters for MIDI synth
    ADSR::Parameters midiSynthEnvelopeParameters;
};
