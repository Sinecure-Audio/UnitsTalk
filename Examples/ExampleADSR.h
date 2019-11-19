#include <chrono>
#include <Units/AmplitudeUnits.h>

// example of an ADSR class and it's declarations.
template<typename NumericType>
class ADSR
{
    // you need to store the samplerate so you can convert samples to seconds if an envelope time changes.
    void setSampleRate(double spec);

    //set the length of an envelope segment.
    void setAttackTime(const std::chrono_duration<NumericType, std::milli>& newAttackTime);
    void setDecayTime(const std::chrono_duration<NumericType, std::milli>& newDecayTime);
    void setReleaseTime(const std::chrono_duration<NumericType, std::milli>& newReleaseTime);

    //set sustain gain
    void setSustainGain(const Decibel<NumericType>& newSustainGain);

    //generate the envelope
    NumericType perform();
    
private:
    //initialize samplerate to 441 by default
    double sampleRate = 44100;
    //duration type for envelope segment times. 
    //You need to store these so you can recalculate envelope coefficients if the samplerate changes.
    std::chrono_duration<NumericType, std::milli> attack{0}, decay{0}, release{0};
    //decibel for the gain of the sustain
    Decibel<FloatType> sustainGain{0};
};