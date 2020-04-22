#include "../Interval/include/Interval.h"
#include "../../Units/include/Units.h"

#include "../Source/DSP/Oscillator.h"
#include "../Source/DSP/Interpolation.h"
#include "../Source/DSP/SmoothedValueBuffer.h"

#include <catch2/catch.hpp>

TEST_CASE("Lerp Function Object", "[Interpolation]") {
	REQUIRE_THAT(LinearInterpolation<double>{}(.5, 1.0, 0.5), Catch::WithinRel(.75));
	REQUIRE_THAT(LinearInterpolation<double>{}(.5, 1.0, 1.0), Catch::WithinRel(1.0));
	REQUIRE_THAT(LinearInterpolation<double>{}(.5, 1.0, 0.0), Catch::WithinRel(.5));
	REQUIRE_THAT(LinearInterpolation<double>{}(-1.0, 1.0, .5), Catch::WithinRel(0.0));
	REQUIRE_THAT(LinearInterpolation<double>{}(1.0, -1.0, .5), Catch::WithinRel(0.0));
	REQUIRE_THAT(LinearInterpolation<double>{}(0.0,  0.0, .5), Catch::WithinRel(0.0));
	REQUIRE_THAT(LinearInterpolation<double>{}(1.0,  1.0, .5), Catch::WithinRel(1.0));
}

//a table that holds a 4096 sample sine wave. reading it with a non-whole number results in an interpolated output.
const auto sineTable =
	[](){
		InterpolatedIntervalArray<double, 4096> arr{};
		auto writer = arr.getWriter();
		constexpr auto rangeMax = arr.size();
		for(size_t i = 0; i < rangeMax; ++i)
			writer[i] = std::sin(2.0*juce::MathConstants<double>::pi
								 *static_cast<double>(i)/static_cast<double>(rangeMax-1));
		return arr;
	}();

const double noiseFloor = Amplitude<double>{-120.0_dB};

//Test that the lerping sin table matches running the samples through linear interpolation
TEST_CASE("Lerp Sin Table", "[Interpolation]") {
	const auto testNumber = GENERATE(range(size_t{0}, (sineTable.size()-1)*8));
	const double index = testNumber/8.0;
	const auto reader = sineTable.getReader();
	REQUIRE_THAT(reader[index], Catch::WithinAbs(LinearInterpolation<double>{}(reader[std::floor(index)], 
																			   reader[std::ceil(index)], 
																			   index-std::floor(index)), 
													noiseFloor));
}

SinOscillator<double> singleTestOscillator{};
bool firstTime = true;

//Test that the output of the oscillator matches the lerped output of the sin table
TEST_CASE("Single Oscillator", "[Oscillators]") {
	constexpr double sampleRate{44100};
	constexpr double frequency{1000};
	if(firstTime) {
		singleTestOscillator.prepare(sampleRate, 32);
		singleTestOscillator.setFrequency(frequency);
		singleTestOscillator.reset();
		firstTime = false;
	}
	const auto reader = sineTable.getReader();
	const auto currentIteration = GENERATE(range(size_t{0}, static_cast<size_t>(44100)));
		
	const double index = fmod(static_cast<double>(currentIteration)*(frequency/sampleRate)*(sineTable.size()-1), sineTable.size()-1);
	const auto referenceValue = reader[index];
	REQUIRE_THAT(singleTestOscillator.perform(), Catch::WithinAbs(referenceValue, noiseFloor));
}

FMPair<SinOscillator<double>> fmPairNoModOscillator{};
bool firstTime2 = true;

//Test that the output of the oscillator matches the lerped output of the sin table
TEST_CASE("FM Pair Single Oscillator", "[Oscillators]") {
	constexpr double sampleRate{44100};
	constexpr double frequency{1000};
	if(firstTime2) {
		fmPairNoModOscillator.prepare(sampleRate, 32);
		fmPairNoModOscillator.setFrequency(frequency);
		fmPairNoModOscillator.setFMRatio(1.0);
		fmPairNoModOscillator.setFMDepth(0.0);
		firstTime2 = false;
	}
	const auto reader = sineTable.getReader();
	const auto currentIteration = GENERATE(range(size_t{0}, static_cast<size_t>(44100)));

	const double index = fmod(static_cast<double>(currentIteration)*(frequency/sampleRate)*(sineTable.size()-1), sineTable.size()-1);
	const auto referenceValue = reader[index];
	REQUIRE_THAT(fmPairNoModOscillator.perform(), Catch::WithinAbs(referenceValue, noiseFloor));
}

template<typename T>
auto periodicMatch(const T& input1, const T& input2) {
	const auto zeroMatcher = Catch::WithinAbs(0.0, noiseFloor);
	const auto oneMatcher = Catch::WithinAbs(1.0, noiseFloor);
	if((zeroMatcher.match(input1) || oneMatcher.match(input2)) || (zeroMatcher.match(input2) && oneMatcher.match(input1)))
		return true;
	else {
		const auto matcher = Catch::WithinAbs(input2, noiseFloor);
		return matcher.match(input1);
	}
}

bool firstTime3 = true;

//Test that the sin oscillator responds to phase modulation correctly
TEST_CASE("Phase Modulated Single Oscillator", "[Oscillators]") {
constexpr double sampleRate{44100};
constexpr double frequency{1000};
if(firstTime3) {
	singleTestOscillator.prepare(sampleRate, 32);
	singleTestOscillator.setFrequency(frequency);
	singleTestOscillator.reset();
	firstTime3 = false;
}
	const auto reader = sineTable.getReader();
	const auto currentIteration = GENERATE(range(size_t{0}, static_cast<size_t>(44100)));

	const double index = fmod(static_cast<double>(currentIteration)*(frequency/sampleRate), 1.0);
	const auto referenceModulator = reader[index];
	const auto modulatedIndex = fmod(index+referenceModulator, 1.0);
	const auto modulatedReadIndex = modulatedIndex*(sineTable.size()-1.0);	
	const auto referenceCarrier = reader[modulatedReadIndex];
	const auto output = singleTestOscillator.perform(referenceModulator);	
	REQUIRE_THAT(output, Catch::WithinAbs(referenceCarrier, noiseFloor));
}

FMPair<SinOscillator<double>> fmPairOscillator{};
bool firstTime4 = true;

//Test the FM oscillator with full modulation
TEST_CASE("FM Pair Depth", "[Oscillators]") {
	constexpr double sampleRate{44100};
	constexpr double frequency{1000};
	if(firstTime4) {
		fmPairOscillator.prepare(sampleRate, 32);
		fmPairOscillator.setFrequency(frequency);
		fmPairOscillator.setFMRatio(1.0);
		fmPairOscillator.setFMDepth(1.0);
		firstTime4 = false;
	}
	const auto reader = sineTable.getReader();
	const auto currentIteration = GENERATE(range(size_t{0}, static_cast<size_t>(44100)));

	const double index = fmod(static_cast<double>(currentIteration)*(frequency/sampleRate), 1.0);
	const auto referenceModulator = reader[index*(sineTable.size()-1.0)];
	const auto modulatedIndex = fmod(index+referenceModulator, 1.0);
	const auto referenceCarrier = reader[modulatedIndex*(sineTable.size()-1.0)];
	const auto output = fmPairOscillator.perform();	
	
	const auto zeroMatcher = Catch::WithinAbs(0.0, noiseFloor);
	const auto oneMatcher = Catch::WithinAbs(1.0, noiseFloor);
	if((zeroMatcher.match(output) || oneMatcher.match(referenceCarrier)) || (zeroMatcher.match(referenceCarrier) && oneMatcher.match(output)))
	//if both the output and the reference signal are 1 and 0 (doesn't matter which is which), we pass.
		REQUIRE(((zeroMatcher.match(output) || oneMatcher.match(referenceCarrier)) || ((zeroMatcher.match(referenceCarrier) && oneMatcher.match(output)))));
	else
	//otherwise, output should be close to reference signal
		REQUIRE_THAT(output, Catch::WithinAbs(referenceCarrier, noiseFloor));
}

//Make sure the smoothed value buffer fills a buffer correctly
TEST_CASE("Smooth", "[Smoothed Value Buffer]") {
    SmoothedValueBuffer<double> smoother{};
	const double sampleRate{44100};
	const int blockSize{128};
    smoother.prepare(sampleRate, blockSize);
	smoother.setSmoothingTime(std::chrono::duration<double>(static_cast<double>(blockSize)/sampleRate));

    smoother.fillBuffer(1.0);
	const size_t currentIteration = GENERATE(range(size_t{0}, static_cast<size_t>(128)));
	REQUIRE_THAT(smoother[currentIteration], Catch::WithinRel(static_cast<double>(1.0+currentIteration)/static_cast<double>(blockSize)));
}

