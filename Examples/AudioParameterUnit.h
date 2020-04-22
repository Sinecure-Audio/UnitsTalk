#pragma once

#include "../Units/include/Units.h"

#include "juce_core/juce_core.h"
#include "juce_audio_processors/juce_audio_processors.h"

using namespace juce;

//A base class for Unit Parameters that allows their conversion to the UnitType they represent
template<typename T>
class UnitParameterBase : public AudioProcessorParameter::Listener
{
public:
    UnitParameterBase(const NormalisableRange<float>& range, const T& initialValue = T{}) :
        internalValue(initialValue), paramRange(range) {}
    
    void parameterValueChanged (int, float newValue) {
        internalValue = T(paramRange.convertFrom0to1(newValue));
    }

    void parameterGestureChanged (int, bool){}
    
    operator const T&() const noexcept{ return internalValue; }
    
private:
    T internalValue{};
    const NormalisableRange<float>& paramRange;
};

// Default class type that is instantiated whenever the underlying parameter type isn't one that we specialize for
// Fails noisely at compile time.
template <typename T, typename U>
class AudioParameterUnit
{
    constexpr AudioParameterUnit() {
        static_assert(std::is_base_of<decltype(*this), UnitParameterBase<T>>::value,
                      "The underlying type for this param should be bool, int, or float!");
    }
};

// Below are several template specializations for juce's AudioParameter types
// These are intended to provide the same functionality as the AudioParameter types they inherit from
// As well as an easy and type safe way to get a unit-typed value out of
template <typename T>
class AudioParameterUnit<T, float> : public AudioParameterFloat, public UnitParameterBase<T>
{
public:
	AudioParameterUnit(const String& parameterID, const String& name,
		NormalisableRange<float> normalisableRange, float defaultValue,
		const String& label = String(), AudioProcessorParameter::Category category = AudioProcessorParameter::genericParameter,
		std::function<String(float value, int maximumStringLength)> stringFromValue = nullptr,
		std::function<float(const String& text)> valueFromString = nullptr) 
		: AudioParameterFloat (parameterID, name, normalisableRange, defaultValue, label, category, stringFromValue, valueFromString),
        UnitParameterBase<T>(getNormalisableRange(), T{defaultValue})
    {addListener(this);}
    
    explicit operator float() = delete;
    
    auto& operator= (const T& newValue){
        AudioParameterFloat::operator= (newValue.count());
        return *this;
    }
};

template <typename T>
class AudioParameterUnit<T, int> : public AudioParameterInt, public UnitParameterBase<T>
{
public:
	AudioParameterUnit(const String& parameterID, const String& name,
		int minValue, int maxValue, int defaultValue,
		const String& label = String(),
		std::function<String(int value, int maximumStringLength)> stringFromInt = nullptr,
		std::function<int(const String& text)> intFromString = nullptr)
		: AudioParameterInt(parameterID, name, minValue, maxValue, defaultValue, label, stringFromInt, intFromString),
        UnitParameterBase<T>(getNormalisableRange(), T{defaultValue})
    {addListener(this);}
    
    explicit operator int() = delete;
    
    auto& operator= (const T& newValue){
        AudioParameterInt::operator= (newValue.count());
        return *this;
    }
};

template <typename T>
class AudioParameterUnit<T, bool> : public AudioParameterBool, public UnitParameterBase<T>
{
public:
	AudioParameterUnit(const String& parameterID, const String& name,
		bool defaultValue,
		const String& label = String(),
		std::function<String(bool value, int maximumStringLength)> stringFromBool = nullptr,
		std::function<bool(const String& text)> boolFromString = nullptr)
		: AudioParameterBool(parameterID, name, defaultValue, label, stringFromBool, boolFromString),
        UnitParameterBase<T>(getNormalisableRange(), T{defaultValue})
    {addListener(this);}
    
    explicit operator bool() = delete;
    
    auto& operator= (const T& newValue){
        AudioParameterBool::operator= (newValue.count());
        return *this;
    }
};
