#pragma once

template <typename T, typename U>
class AudioParameterUnit {};

template <typename T>
class AudioParameterUnit<T, float> : public AudioParameterFloat
{
	public:
	AudioParameterUnit(const String& parameterID, const String& name,
		NormalisableRange<float> normalisableRange, float defaultValue,
		const String& label = String(), AudioProcessorParameter::Category category = AudioProcessorParameter::genericParameter,
		std::function<String(float value, int maximumStringLength)> stringFromValue = nullptr,
		std::function<float(const String& text)> valueFromString = nullptr) 
		: AudioParameterFloat (parameterID, name, normalisableRange, defaultValue, label, category, stringFromValue, valueFromString) 
	{}

	operator T() const noexcept {
		return T(get());
	}
};

template <typename T>
class AudioParameterUnit<T, int> : public AudioParameterInt
{
public:
	AudioParameterUnit(const String& parameterID, const String& name,
		int minValue, int maxValue, int defaultValue,
		const String& label = String(),
		std::function<String(int value, int maximumStringLength)> stringFromInt = nullptr,
		std::function<int(const String& text)> intFromString = nullptr)
		: AudioParameterInt(parameterID, name, minValue, maxValue, defaultValue, label, stringFromInt, intFromString) {}

	operator T() const noexcept{
		return T(get());
	}
};

template <typename T>
class AudioParameterUnit<T, bool> : public AudioParameterBool
{
public:
	AudioParameterUnit(const String& parameterID, const String& name,
		bool defaultValue,
		const String& label = String(),
		std::function<String(bool value, int maximumStringLength)> stringFromBool = nullptr,
		std::function<bool(const String& text)> boolFromString = nullptr)
		: AudioParameterBool(parameterID, name, defaultValue, label, stringFromBool, boolFromString) {}

	operator T()const noexcept{
		return T(get());
	}
};