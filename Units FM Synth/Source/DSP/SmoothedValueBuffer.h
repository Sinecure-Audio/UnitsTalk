#pragma once

#include <chrono>

//needed for tests
#include <juce_audio_basics/juce_audio_basics.h>

// A class that writes a series of values from a juce::SmoothedValue into a buffer
// This allows each voice in the synthesizer to access smoothed parameter values
// without making many smoothers
template<typename NumericType>
class SmoothedValueBuffer
{
public:
    SmoothedValueBuffer(){smoother.setCurrentAndTargetValue(NumericType{0});}
    
    void prepare(const double& newSampleRate, const int& bufferSize) {
        if(static_cast<size_t>(bufferSize) > buffer.size())
            buffer.resize(static_cast<size_t>(bufferSize));
        sampleRate = newSampleRate;
        smoother.reset(sampleRate, smoothingTime.count());
    }
    
    void setSmoothingTime(const std::chrono::duration<NumericType>& newSmoothingTime){
        smoothingTime=newSmoothingTime;
        smoother.reset(sampleRate, smoothingTime.count());
    }
    
    void reset() {std::fill(buffer.begin(), buffer.end(), 0);}
    
    void fillBuffer(const NumericType& newTargetValue) {
        if(newTargetValue != previousTargetValue) {
            smoother.setTargetValue(newTargetValue);
            previousTargetValue = newTargetValue;
        }
        fillBuffer();
    }
    
    void fillBuffer() {
        for(auto&& sample : buffer)
            sample = smoother.getNextValue();
    }
    
    const NumericType& operator[](const size_t& index) const noexcept {return buffer[index];}
    
private:
    std::vector<NumericType> buffer{};
    NumericType previousTargetValue{};
    juce::SmoothedValue<NumericType, juce::ValueSmoothingTypes::Linear> smoother{};
    double sampleRate{44100.0};
    std::chrono::duration<double> smoothingTime{0};
};
