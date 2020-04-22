#include <catch2/catch.hpp>

#include "../../Examples/AudioParameterUnit.h"

//Two tests that make sure we can get a functioning unit typed reference from an AudioParameterUnit

TEST_CASE("Float Parameter", "[Parameter To Unit Conversions]") {
    AudioParameterUnit<Decibel<float>, float> dbFloatParam{"", "", {0.0f, 10.0f, .001f}, 5.0f};

    const Decibel<float>& dbFloat = dbFloatParam;

	REQUIRE_THAT(dbFloatParam.get(), Catch::WithinRel(5.0f));
    REQUIRE_THAT(dbFloat.count(), Catch::WithinRel(5.0f));

    dbFloatParam = Decibel<float>{1.0f};

    REQUIRE_THAT(dbFloatParam.get(), Catch::WithinRel(1.0f));
    REQUIRE_THAT(dbFloat.count(), Catch::WithinRel(1.0f));
}

TEST_CASE("Int Parameter", "[Parameter To Unit Conversions]") {
    AudioParameterUnit<Decibel<int>, int> dbIntParam{"", "", 0, 10, 5};
    
    const Decibel<int>& dbInt = dbIntParam;

    REQUIRE(dbIntParam.get() == 5);
	REQUIRE(dbInt == Decibel<int>{5});

    dbIntParam = Decibel<int>{1};

    REQUIRE(dbIntParam.get() == 1);
	REQUIRE(dbInt == Decibel<int>{1});
}