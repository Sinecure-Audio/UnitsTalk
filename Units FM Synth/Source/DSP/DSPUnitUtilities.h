#pragma once

#include "Oscillator.h"

#include "../../../Units/include/Units.h"

#include <juce_dsp/juce_dsp.h>
using namespace juce;


// A struct that holds the values for an adsr as units.
// Using this as a normal ADSR::Parameters will create an equivalent ADSR::Parameters struct
struct TimedADSRParameters
{
    std::chrono::duration<float> attack{std::chrono::duration<float>{ 0.0 }},
                                 decay{std::chrono::duration<float>{ 0.0f }};
    Amplitude<float>             sustain{Amplitude<float>(1.0f)};
    std::chrono::duration<float> release{std::chrono::duration<float>{ 0.0f }};
    
    operator ADSR::Parameters() const {return ADSR::Parameters{attack.count(), decay.count(), sustain.count(), release.count()};}
};

//A simple wrapper for dsp::StateVariableFilter::Parameters coefficients that asserts if you attempt to use it without units.
template<typename NumericType>
struct SVFParams : public dsp::StateVariableFilter::Parameters<NumericType>
{
	void setCutOffFrequency(const double& sampleRate, const NumericType& frequency, const QCoefficient<NumericType>& Q = QCoefficient<NumericType>(NumericType{1} / MathConstants<NumericType>::sqrt(2))) noexcept {
		dsp::StateVariableFilter::Parameters<NumericType>::setCutOffFrequency(sampleRate, frequency, Q.count());
	}

	void setCutOffFrequency(double sampleRate, NumericType frequency, NumericType Q = static_cast<NumericType>(1.0 / MathConstants< NumericType >::sqrt(2))) {
        static_assert(std::is_same<decltype(Q), QCoefficient<NumericType>>::value,
                      "Use units for your resonance parameter instead of numeric types!");
	}
};

//A simple wrapper for dsp::IIR coefficients that asserts if you attempt to use it without units.
template<typename NumericType>
struct UnitsIIRCoefficients : public dsp::IIR::Coefficients<NumericType>
{
    static typename dsp::IIR::Coefficients<NumericType>::Ptr makeLowPass(const double& sampleRate, const NumericType& frequency, const QCoefficient<NumericType>& Q) {
        return dsp::IIR::Coefficients<NumericType>::makeLowPass(sampleRate, frequency, Q.count());
    }

    static typename dsp::IIR::Coefficients<NumericType>::Ptr makeHighPass(const double& sampleRate, const NumericType& frequency, const QCoefficient<NumericType>& Q) {
        return dsp::IIR::Coefficients<NumericType>::makeHighPass(sampleRate, frequency, Q.count());
    }
    
    static typename dsp::IIR::Coefficients<NumericType>::Ptr makeBandPass(const double& sampleRate, const NumericType& frequency, const QCoefficient<NumericType>& Q) {
        return dsp::IIR::Coefficients<NumericType>::makeBandPass(sampleRate, frequency, Q.count());
    }
    
    static typename dsp::IIR::Coefficients<NumericType>::Ptr makeNotch(const double& sampleRate, const NumericType& frequency, const QCoefficient<NumericType>& Q) {
        return dsp::IIR::Coefficients<NumericType>::makeNotch(sampleRate, frequency, Q.count());
    }
    
    static typename dsp::IIR::Coefficients<NumericType>::Ptr makeAllPass(const double& sampleRate, const NumericType& frequency, const QCoefficient<NumericType>& Q) {
        return dsp::IIR::Coefficients<NumericType>::makeAllPass(sampleRate, frequency, Q.count());
    }
    
    static typename dsp::IIR::Coefficients<NumericType>::Ptr makeLowShelf(const double& sampleRate, const NumericType& frequency, const QCoefficient<NumericType>& Q, Amplitude<NumericType> filterAmplitude) {
        return dsp::IIR::Coefficients<NumericType>::makeLowShelf(sampleRate, frequency, Q.count(), filterAmplitude.count());
    }
    
    static typename dsp::IIR::Coefficients<NumericType>::Ptr makeHighShelf(const double& sampleRate, const NumericType& frequency, const QCoefficient<NumericType>& Q, Amplitude<NumericType> filterAmplitude) {
        return dsp::IIR::Coefficients<NumericType>::makeHighShelf(sampleRate, frequency, Q.count(), filterAmplitude.count());
    }
    
    static typename dsp::IIR::Coefficients<NumericType>::Ptr makePeakFilter(const double& sampleRate, const NumericType& frequency, const QCoefficient<NumericType>& Q, Amplitude<NumericType> filterAmplitude) {
        return dsp::IIR::Coefficients<NumericType>::makePeakFilter(sampleRate, frequency, Q.count(), filterAmplitude.count());
    }




//============================================================
// Unitless Function Aliases
//============================================================
    static typename dsp::IIR::Coefficients<NumericType>::Ptr makeLowPass(double sampleRate, NumericType frequency, NumericType Q) {
        static_assert(std::is_same<decltype(Q), QCoefficient<NumericType>>::value,
        "Use units for your resonance parameter instead of numeric types!");
        return dsp::IIR::Coefficients<NumericType>::makeLowPass(sampleRate, frequency, Q);
    }

    static typename dsp::IIR::Coefficients<NumericType>::Ptr makeHighPass(double sampleRate, NumericType frequency, NumericType Q) {
        static_assert(std::is_same<decltype(Q), QCoefficient<NumericType>>::value,
        "Use units for your resonance parameter instead of numeric types!");
        return dsp::IIR::Coefficients<NumericType>::makeHighPass(sampleRate, frequency, Q);
    }
    
    static typename dsp::IIR::Coefficients<NumericType>::Ptr makeBandPass(double sampleRate, NumericType frequency, NumericType Q) {
        static_assert(std::is_same<decltype(Q), QCoefficient<NumericType>>::value,
        "Use units for your resonance parameter instead of numeric types!");
        return dsp::IIR::Coefficients<NumericType>::makeBandPass(sampleRate, frequency, Q);
    }
    
    static typename dsp::IIR::Coefficients<NumericType>::Ptr makeNotch(double sampleRate, NumericType frequency, NumericType Q) {
        static_assert(std::is_same<decltype(Q), QCoefficient<NumericType>>::value,
        "Use units for your resonance parameter instead of numeric types!");
        return dsp::IIR::Coefficients<NumericType>::makeNotch(sampleRate, frequency, Q);
    }
    
    static typename dsp::IIR::Coefficients<NumericType>::Ptr makeAllPass(double sampleRate, NumericType frequency, NumericType Q) {
        static_assert(std::is_same<decltype(Q), QCoefficient<NumericType>>::value,
        "Use units for your resonance parameter instead of numeric types!");
        return dsp::IIR::Coefficients<NumericType>::makeAllPass(sampleRate, frequency, Q);
    }
    
    static typename dsp::IIR::Coefficients<NumericType>::Ptr makeLowShelf(double sampleRate, NumericType frequency, NumericType Q, NumericType filterAmplitude) {
        static_assert(std::is_same<decltype(Q), QCoefficient<NumericType>>::value,
        "Use units for your resonance parameter instead of numeric types!");
        return dsp::IIR::Coefficients<NumericType>::makeLowShelf(sampleRate, frequency, Q, filterAmplitude);
    }
    
    static typename dsp::IIR::Coefficients<NumericType>::Ptr makeHighShelf(double sampleRate, NumericType frequency, NumericType Q, NumericType filterAmplitude) {
        static_assert(std::is_same<decltype(Q), QCoefficient<NumericType>>::value,
        "Use units for your resonance parameter instead of numeric types!");
        return dsp::IIR::Coefficients<NumericType>::makeHighShelf(sampleRate, frequency, Q, filterAmplitude);
    }
    
    static typename dsp::IIR::Coefficients<NumericType>::Ptr makePeakFilter(double sampleRate, NumericType frequency, NumericType Q, NumericType filterAmplitude) {
		static_assert(std::is_same<decltype(Q), QCoefficient<NumericType>>::value,
        "Use units for your resonance parameter instead of numeric types!");
        return dsp::IIR::Coefficients<NumericType>::makePeakFilter(sampleRate, frequency, Q, filterAmplitude);
    }
};
