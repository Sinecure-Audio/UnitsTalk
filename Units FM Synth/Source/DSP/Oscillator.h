#pragma once

#include "Interpolation.h"
#include "../../Interval/include/Interval.h"

#include <juce_core/juce_core.h>

// A simple FM Oscillator class. Uses an Interval to hold a phase
// Then, this phase is used with an interpolated IntervalIndexedArray to get a lerped output signal
template<typename InputType>
class SinOscillator
{
public:
    using NumericType = InputType;
    
    void prepare(const double& newSampleRate, const int&) {
        performRate = newSampleRate;
        setFrequency(frequency);
        reset();
    }
    
    void reset() { phase = 0; }
    
    void setFrequency(const NumericType& newFrequency) noexcept {
        frequency = newFrequency;
        angularVelocity = frequency/performRate;
    }
    
    auto perform() noexcept {
        const auto output = wavetable.getReader()[phase.getValue()*(wavetable.size()-1)];
        //At time 0 a sinwave is 0, so we should get the value from the table THEN increment the phase
        phase += angularVelocity;
        return output;
    }

//   This is the ideal way to perform, but due to a bug in the interval library it is imprecise
//    auto perform() noexcept {
//        const auto output =  wavetable.getReader()[phase];
//        phase += angularVelocity;
//        return output;
//    }
    
    auto perform(const NumericType& phaseInput) noexcept {
        const auto output = wavetable.getReader()[(phase.getValue()+phaseInput)*(wavetable.size()-1)];
        phase += angularVelocity;
        return output;
    }
    
//   This is the ideal way to perform, but due to a bug in the interval library it is imprecise
//    auto perform(const NumericType& phaseInput) noexcept {
//        const auto output =  wavetable.getReader()[phase+phaseInput];
//        phase += angularVelocity;
//        return output;
//    }
    
private:
    //hold the phase in a type that will automatically wrap the values greater than 1 around 0.
    INTERVAL_TYPE(NumericType{0}, NumericType{1}, IntervalWrapModes::Wrap) phase{0.0};
    
    NumericType angularVelocity{0}, frequency{0};
    double performRate{44100};
    //Generate a sinewave table for all instances of the class
    static const inline InterpolatedIntervalArray<NumericType, 4096> wavetable =
        [](){
            InterpolatedIntervalArray<NumericType, 4096> arr{};
            auto writer = arr.getWriter();
            for(size_t i = 0; i < arr.size(); ++i) {
                writer[i] = std::sin(NumericType{2}
                                     *juce::MathConstants<NumericType>::pi
                                     *static_cast<NumericType>(i)
                                     /static_cast<NumericType>(arr.size()-1));
            }
            return arr;
        }();
};

//A struct that holds two oscillators, making them act as an FM pair
template<typename CarrierOscillator, typename ModulationOscillator = CarrierOscillator>
struct FMPair
{
    using NumericType = typename CarrierOscillator::NumericType;
    
    void prepare(const double& newPerformRate, const int& newBlockSize) {
        carrierOsc.prepare(newPerformRate, newBlockSize);
        modOsc.prepare(newPerformRate, newBlockSize);
    }
    
    void reset() {
        carrierOsc.reset();
        modOsc.reset();
    }
    
    void setFrequency(const NumericType& newFrequency) noexcept {
        frequency = newFrequency;
        modOsc.setFrequency(frequency*fmRatio);
        carrierOsc.setFrequency(frequency);
    }
    
    void setFMDepth(const NumericType& newFMDepth) noexcept {
        fmDepth = newFMDepth;
    }
    
    void setFMRatio(const NumericType& newFMRatio) noexcept {
        fmRatio = newFMRatio;
        modOsc.setFrequency(frequency*fmRatio);
    }
    
    auto perform() noexcept {
        return carrierOsc.perform(modOsc.perform()*fmDepth);
    }
    
private:
    CarrierOscillator carrierOsc;
    ModulationOscillator modOsc;
    
    NumericType fmOffset{};
    
    NumericType frequency{220}, fmDepth{0}, fmRatio{1};
};
