#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <iostream>

#include "core/generatives/generator_RM.h"
#include "core/generatives/variable.h"
#include "core/generatives/OLD_oscillator.h"

class BasicGenerator {
public:
    BasicGenerator()
    : osc_type("", handler, Oscillator::Type::phasor)
    , freq("", handler, 0.25f)
    , mul("", handler, 1.0f)
    , add("", handler, 0.0f)
    , duty("", handler, 0.5f)
    , curve("", handler, 1.0f)
    , osc_enabled("", handler, true)
    , generator_enabled("", handler, true)
    , sequence("", handler)
    , interp("", handler, InterpolationStrategy<double>())
    , oscillator("", handler, &osc_type, &freq, &add, &mul, &duty, &curve, &osc_enabled)
    , generator("", handler, &oscillator, &interp, &sequence, &generator_enabled) {}

    ParameterHandler handler;

    Variable<Oscillator::Type> osc_type;
    Variable<float> freq;
    Variable<float> mul;
    Variable<float> add;
    Variable<float> duty;
    Variable<float> curve;
    Variable<bool> osc_enabled;

    Variable<bool> generator_enabled;

    Sequence<double> sequence;
    Variable<InterpolationStrategy<double>> interp;
    Oscillator oscillator;


    Generator<double> generator;
};


TEST_CASE("Generator ctor") {
    auto generator = BasicGenerator();
}

TEST_CASE("Unity Phasor Stepped") {
    auto generator = BasicGenerator();
    generator.generator.set_sequence(nullptr);

    generator.osc_type.set_value(Oscillator::Type::phasor);
    generator.freq.set_value(0.1f);

    TimePoint t;
    auto res = generator.generator.process(t);
    REQUIRE(!res.empty());
    REQUIRE_THAT(res.at(0.0), Catch::Matchers::WithinAbs(0.0, 1e-5));

    REQUIRE_THAT(generator.generator.process(t).at(0), Catch::Matchers::WithinAbs(0.1, 1e-5));
    REQUIRE_THAT(generator.generator.process(t).at(0), Catch::Matchers::WithinAbs(0.2, 1e-5));
    REQUIRE_THAT(generator.generator.process(t).at(0), Catch::Matchers::WithinAbs(0.3, 1e-5));

    for (int i = 0; i < 6; ++i) {
        generator.generator.process(t);
    }

    REQUIRE_THAT(generator.generator.process(t).at(0), Catch::Matchers::WithinAbs(0.0, 1e-5));
    REQUIRE_THAT(generator.generator.process(t).at(0), Catch::Matchers::WithinAbs(0.1, 1e-5));
}


TEST_CASE("Unity Looper") {
    auto generator = BasicGenerator();
    auto seq = std::vector<double>{60.0, 61.0, 62.0, 63.0};
    generator.sequence.get_parameter_obj().reset_values(seq);
    generator.osc_type.set_value(Oscillator::Type::phasor);
    generator.freq.set_value(1.0f / static_cast<float>(seq.size()));

    TimePoint t;

    REQUIRE_THAT(generator.generator.process(t).at(0), Catch::Matchers::WithinAbs(60, 1e-8));
    REQUIRE_THAT(generator.generator.process(t).at(0), Catch::Matchers::WithinAbs(61, 1e-8));
    REQUIRE_THAT(generator.generator.process(t).at(0), Catch::Matchers::WithinAbs(62, 1e-8));
    REQUIRE_THAT(generator.generator.process(t).at(0), Catch::Matchers::WithinAbs(63, 1e-8));
    REQUIRE_THAT(generator.generator.process(t).at(0), Catch::Matchers::WithinAbs(60, 1e-8));
    REQUIRE_THAT(generator.generator.process(t).at(0), Catch::Matchers::WithinAbs(61, 1e-8));
}
//
//
//// ==============================================================================================
//
//TEST_CASE("Generator as Looper") {
//    Mapping<int> mapping(std::vector{0, 2, 4, 5, 7, 9, 11});
//
//    SECTION("Linear forward-looping") {
//        auto step_size = 1.0 / static_cast<double>(mapping.size());
//
//        Generator<int> generator{step_size
//                                 , 0.0
//                                 , Phasor::Mode::stepped
//                                 , std::make_unique<Identity>()
//                                 , std::make_unique<Mapping<int>>(mapping)};
//
//        for (int i = 0; i < 100; ++i) {
//            REQUIRE(generator.process(TimePoint()) == mapping.at(static_cast<std::size_t>(i) % mapping.size()));
//        }
//    }
//
//    SECTION("Linear backward-looping") {
//        auto step_size = -1.0 / static_cast<double>(mapping.size());
//
//        Generator<int> generator{step_size
//                                 , 0.0
//
//                                 , Phasor::Mode::stepped
//                                 , std::make_unique<Identity>()
//                                 , std::make_unique<Mapping<int>>(mapping)};
//
//        for (long i = 0; i < 100; ++i) {
//            REQUIRE(generator.process(TimePoint()) ==
//                    mapping.at(static_cast<std::size_t>(utils::modulo(-i, static_cast<long>(mapping.size())))));
//        }
//    }
//}
//
