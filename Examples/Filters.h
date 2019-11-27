#include "../Units/FilterUnits.h"

//A simple wrapper for dsp::juce::StateVariableFilter::Parameters that triggers an assert if you don't use unit types.
template<typename NumericType>
struct UnitsSVFParams : public dsp::StateVariableFilter::Parameters<NumericType>
{
	void setCutOffFrequency(const double& sampleRate, const NumericType& frequency, const ResonanceCoefficient<NumericType>& resonance = QCoefficient<NumericType>{NumericType{1} / MathConstants< NumericType >::sqrt(2)}) noexcept {
		dsp::StateVariableFilter::Parameters<NumericType>::setCutOffFrequency(std::forward<const double>(sampleRate), std::forward<const NumericType>(frequency), resonance.count());
	}

	void setCutOffFrequency(double sampleRate, NumericType frequency, NumericType resonance = NumericType{1} / MathConstants< NumericType >::sqrt(2)) {
		jassertfalse;// "Use units for your resonance parameter instead of numeric types!");
	}
};

//A simple wrapper for dsp::juce::IIR coefficients that triggers an assert if you don't use unit types.
template<typename NumericType>
struct UnitsIIRCoefficients : public dsp::IIRCoefficients
{
    static IIRCoefficients makeLowPass(double&& sampleRate, NumericType&& frequency, QCoefficient<NumericType> Q) {
        return IIRCoefficients::makeLowPass(std::forward<double>(sampleRate), std::forward<NumericType>(frequency), Q.count())
    }

    static IIRCoefficients makeHighPass(double&& sampleRate, NumericType&& frequency, QCoefficient<NumericType> Q) {
        return IIRCoefficients::makeHighPass(std::forward<double>(sampleRate), std::forward<NumericType>(frequency), Q.count())
    }
    
    static IIRCoefficients makeBandPass(double&& sampleRate, NumericType&& frequency, QCoefficient<NumericType> Q) {
        return IIRCoefficients::makeBandPass(std::forward<double>(sampleRate), std::forward<NumericType>(frequency), Q.count())
    }
    
    static IIRCoefficients makeNotchFilter(double&& sampleRate, NumericType&& frequency, QCoefficient<NumericType> Q) {
        return IIRCoefficients::makeNotchFilter(std::forward<double>(sampleRate), std::forward<NumericType>(frequency), Q.count())
    }
    
    static IIRCoefficients makeAllPass(double&& sampleRate, double&& frequency, QCoefficient<NumericType> Q) {
        return IIRCoefficients::makeAllPass(std::forward<double>(sampleRate), std::forward<NumericType>(frequency), Q.count())
    }
    
    static IIRCoefficients makeLowShelf(double&& sampleRate, double&& frequency, QCoefficient<NumericType> Q) {
        return IIRCoefficients::makeLowShelf(std::forward<double>(sampleRate), std::forward<NumericType>(frequency), Q.count())
    }
    
    static IIRCoefficients makeHighShelf(double&& sampleRate, double&& frequency, QCoefficient<NumericType> Q) {
        return IIRCoefficients::makeHighShelf(std::forward<double>(sampleRate), std::forward<NumericType>(frequency), Q.count())
    }
    
    static IIRCoefficients makePeakFilter(double&& sampleRate, double&& frequency, QCoefficient<NumericType> Q) {
        return IIRCoefficients::makePeakFilter(std::forward<double>(sampleRate), std::forward<NumericType>(frequency), Q.count())
    }




//============================================================
// Unitless Function Aliases
//============================================================
    static IIRCoefficients makeLowPass(double sampleRate, NumericType frequency, NumericType Q) {
        jassertfalse;// "Use units for your resonance parameter instead of numeric types!");
        return IIRCoefficients::makeLowPass(sampleRate, frequency, Q)
    }

    static IIRCoefficients makeHighPass(double sampleRate, NumericType frequency, NumericType Q) {
        jassertfalse;// "Use units for your resonance parameter instead of numeric types!");
        return IIRCoefficients::makeHighPass(sampleRate, frequency, Q)
    }
    
    static IIRCoefficients makeBandPass(double sampleRate, NumericType frequency, NumericType Q) {
        jassertfalse;// "Use units for your resonance parameter instead of numeric types!");
        return IIRCoefficients::makeBandPass(sampleRate, frequency, Q)
    }
    
    static IIRCoefficients makeNotchFilter(double sampleRate, NumericType frequency, NumericType Q) {
        jassertfalse;// "Use units for your resonance parameter instead of numeric types!");
        return IIRCoefficients::makeNotchFilter(sampleRate, frequency, Q)
    }
    
    static IIRCoefficients makeAllPass(double sampleRate, NumericType frequency, NumericType Q) {
        jassertfalse;// "Use units for your resonance parameter instead of numeric types!");
        return IIRCoefficients::makeAllPass(sampleRate, frequency, Q)
    }
    
    static IIRCoefficients makeLowShelf(double sampleRate, NumericType frequency, NumericType Q) {
        jassertfalse;// "Use units for your resonance parameter instead of numeric types!");
        return IIRCoefficients::makeLowShelf(sampleRate, frequency, Q)
    }
    
    static IIRCoefficients makeHighShelf(double sampleRate, NumericType frequency, NumericType Q) {
        jassertfalse;// "Use units for your resonance parameter instead of numeric types!");
        return IIRCoefficients::makeHighShelf(sampleRate, frequency, Q)
    }
    
    static IIRCoefficients makePeakFilter(double sampleRate, NumericType frequency, NumericType Q) {
		jassertfalse;// "Use units for your resonance parameter instead of numeric types!");
        return IIRCoefficients::makePeakFilter(sampleRate, frequency, Q)
    }
    
};

class UnitsFilterAudioProcessor : public AudioProcessor
{
//...


private:
    const double incrementAmount{.2};
    bool isFilterDeltaPositive{true};


    ReferenceCountedObjectPtr<UnitsSVFParams<double>> filterParams = new SVFFilterParams<double>();
    // ReferenceCountedObjectPtr<UnitsIIRCoefficients<double>> filterParams{};

    dsp::StateVariableFilter<double> filter(filterParams);
    //dsp::IIRFilter<double> filter;

};
