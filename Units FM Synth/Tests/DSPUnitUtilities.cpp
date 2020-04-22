#include <catch2/catch.hpp>

#include "../Source/DSP/DSPUnitUtilities.h"

//Test the units ADSR to see if it initializes values and converts dB to amplitude correctly
TEST_CASE("ADSR", "[Utilities]") {
    TimedADSRParameters unitADSRParams{std::chrono::seconds{1}, std::chrono::seconds{1}, 0.0_dB, std::chrono::seconds{1}};
    juce::ADSR::Parameters normalADSRParams{1,1,1,1};
    
	REQUIRE_THAT(normalADSRParams.attack,  Catch::WithinRel(unitADSRParams.attack.count()));
    REQUIRE_THAT(normalADSRParams.decay,   Catch::WithinRel(unitADSRParams.decay.count()));
    REQUIRE_THAT(normalADSRParams.sustain, Catch::WithinRel(Amplitude<float>(unitADSRParams.sustain).count()));
    REQUIRE_THAT(normalADSRParams.release, Catch::WithinRel(unitADSRParams.release.count()));
}

//Test the units SVF params and make sure they create the same set of coefficients as the juce SVF params
TEST_CASE("SVF", "[Utilities]") {
    SVFParams<double> unitParams{};
    juce::dsp::StateVariableFilter::Parameters<double> juceParams{};

    unitParams.setCutOffFrequency(44100.0, 1000.0, QCoefficient<double>{2.0});
    juceParams.setCutOffFrequency(44100.0, 1000.0, 2.0);
    
	REQUIRE_THAT(juceParams.g,  Catch::WithinRel(unitParams.g));
    REQUIRE_THAT(juceParams.R2, Catch::WithinRel(unitParams.R2));
    REQUIRE_THAT(juceParams.h,  Catch::WithinRel(unitParams.h));
}

//Test the units IIR coefficients and make sure they create the same set of coefficients as the juce IIR Coefficients
TEST_CASE("IIR", "[Utilities]") {
    UnitsIIRCoefficients<double> unitsCoefficients{};
    dsp::IIR::Coefficients<double> juceCoefficients{};

    constexpr double sampleRate{44100.0};
    constexpr double frequency{1000.0};
    constexpr double qAmount{2.0};

    auto a = unitsCoefficients.makeLowPass(sampleRate, frequency, QCoefficient<double>{qAmount});
    auto b = juceCoefficients .makeLowPass(sampleRate, frequency, qAmount);
    REQUIRE(a->coefficients == b->coefficients);

    a = unitsCoefficients.makeHighPass(sampleRate, frequency, QCoefficient<double>{qAmount});
    b = juceCoefficients .makeHighPass(sampleRate, frequency, qAmount);
	REQUIRE(a->coefficients == b->coefficients);

    a = unitsCoefficients.makeBandPass(sampleRate, frequency, QCoefficient<double>{qAmount});
    b = juceCoefficients .makeBandPass(sampleRate, frequency, qAmount);
	REQUIRE(a->coefficients == b->coefficients);

    a = unitsCoefficients.makeNotch(sampleRate, frequency, QCoefficient<double>{qAmount});
    b = juceCoefficients .makeNotch(sampleRate, frequency, qAmount);
	REQUIRE(a->coefficients == b->coefficients);

    a = unitsCoefficients.makeAllPass(sampleRate, frequency, QCoefficient<double>{qAmount});
    b = juceCoefficients .makeAllPass(sampleRate, frequency, qAmount);
	REQUIRE(a->coefficients == b->coefficients);

    a = unitsCoefficients.makeLowShelf(sampleRate, frequency, QCoefficient<double>{qAmount}, 1.0);
    b = juceCoefficients .makeLowShelf(sampleRate, frequency, qAmount, 1.0);
	REQUIRE(a->coefficients == b->coefficients);

    a = unitsCoefficients.makeHighShelf(sampleRate, frequency, QCoefficient<double>{qAmount}, 1.0);
    b = juceCoefficients .makeHighShelf(sampleRate, frequency, qAmount, 1.0);
	REQUIRE(a->coefficients == b->coefficients);

    a = unitsCoefficients.makePeakFilter(sampleRate, frequency, QCoefficient<double>{qAmount}, 1.0);
    b = juceCoefficients .makePeakFilter(sampleRate, frequency, qAmount, 1.0);
	REQUIRE(a->coefficients == b->coefficients);
}