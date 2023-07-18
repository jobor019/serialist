#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "phasor.h"
#include "variable.h"
#include "oscillator.h"
#include "parameter_policy.h"

#include <thread>
#include <cmath>

class OscillatorWrapper {
public:
    OscillatorWrapper()
            : osc_type("", handler, Oscillator::Type::phasor)
              , freq("", handler, 0.25f)
              , mul("", handler, 1.0f)
              , add("", handler, 0.0f)
              , duty("", handler, 0.5f)
              , curve("", handler, 1.0f)
              , osc_enabled("", handler, true)
              , oscillator("", handler, &osc_type, &freq, &add, &mul, &duty, &curve, &osc_enabled) {}


    ParameterHandler handler;

    Variable<Oscillator::Type> osc_type;
    Variable<float> freq;
    Variable<float> mul;
    Variable<float> add;
    Variable<float> duty;
    Variable<float> curve;
    Variable<bool> osc_enabled;

    Oscillator oscillator;
};


TEST_CASE("m_phasor stepped", "[m_phasor]") {

    SECTION("unity m_phasor") {
        double gain = 0.1;
        double max = 1.0;
        Phasor p{gain, max, 0.0, Phasor::Mode::stepped, 0.0};

        double x;
        for (int i = 0; i < 100; ++i) {
            x = p.process(0);
            REQUIRE_THAT(x, Catch::Matchers::WithinAbs(std::fmod(gain * i, max), 1e-8));
            REQUIRE(x < max);
        }
    }


    SECTION("Non-unity maximum") {
        double gain = 0.001;
        double max = 4.5;
        Phasor p{gain, max, 0.0, Phasor::Mode::stepped, 0.0};

        double x;
        for (int i = 0; i < 100; ++i) {
            x = p.process(0);
            REQUIRE_THAT(x, Catch::Matchers::WithinAbs(std::fmod(gain * i, max), 1e-8));
            REQUIRE(x < max);
        }
    }

    SECTION("Negative step size") {
        double gain = -0.1;
        double max = 1.0;
        Phasor p{gain, max, 0.0, Phasor::Mode::stepped, 0.0};


        double x;
        double y = 0;
        for (int i = 0; i < 100; ++i) {
            x = p.process(0);
            if (y < 0) {
                y += max;
            }
            REQUIRE_THAT(x, Catch::Matchers::WithinAbs(y, 1e-8));
            REQUIRE(x < max);
            REQUIRE(x >= 0);
            y += gain;
        }
    }

    SECTION("Variable step size") {
        double max = 1.0;
        Phasor p{0.1, max, 0.0, Phasor::Mode::stepped, 0.0};
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.0, 1e-8));
        p.set_step_size(0.2);
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.2, 1e-8));
        p.set_step_size(0.8);
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.0, 1e-8));
    }

    SECTION("Variable phase") {
        double max = 1.0;
        Phasor p{0.1, max, 0.5, Phasor::Mode::stepped, 0.0};
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.5, 1e-8));
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.6, 1e-8));
        p.set_phase(0.2, true);
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.2, 1e-8));
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.3, 1e-8));
        p.set_phase(0.9, true);
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.9, 1e-8));
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.0, 1e-8));
    }
}


TEST_CASE("Oscillator Ctor") {
    auto oscillator = OscillatorWrapper();
}


TEST_CASE("Unity Phasor") {
    auto oscillator = OscillatorWrapper();
    oscillator.osc_type.set_value(Oscillator::Type::phasor);
    oscillator.freq.set_value(0.3f);

    TimePoint t;

    REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.0, 1e-5));
    REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.3, 1e-5));
    REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.6, 1e-5));
    REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.9, 1e-5));
    REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.2, 1e-5));
    REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.5, 1e-5));
}


TEST_CASE("Unity Phasor Drifting") {
    auto oscillator = OscillatorWrapper();
    oscillator.osc_type.set_value(Oscillator::Type::phasor);
    oscillator.freq.set_value(0.1f);

    TimePoint t;

    // TODO: Obvious drifting issues here
    for (int i = 0; i < 10000; ++i) {
        auto v = oscillator.oscillator.process(t).at(0);
        std::cout << i << ": " << v << "\n";
        REQUIRE_THAT(v, Catch::Matchers::WithinAbs((i % 10) / 10.0f, 1e-5));
    }
}


TEST_CASE("Sin Oscillator with Add Value") {
    auto oscillator = OscillatorWrapper();
    oscillator.osc_type.set_value(Oscillator::Type::sin);
    oscillator.freq.set_value(0.25f);
    oscillator.add.set_value(2.0f);

    TimePoint t;

    REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(2.0, 1e-5));
    REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(2.5, 1e-5));
    REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(3.0, 1e-5));
    REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(2.5, 1e-5));
    REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(2.0, 1e-5));
}


TEST_CASE("Phasor Oscillator") {
    OscillatorWrapper oscillator;
    oscillator.osc_type.set_value(Oscillator::Type::phasor);
    oscillator.freq.set_value(0.25f);

    TimePoint t;

    SECTION("Test Uniform") {
        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.0, 1e-5));
        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.25, 1e-5));
        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.5, 1e-5));
        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.75, 1e-5));
        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.0, 1e-5));
    }

    SECTION("Test Freq and Mul") {
        oscillator.freq.set_value(0.5f);
        oscillator.mul.set_value(0.5f);

        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.0, 1e-5));
        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.25, 1e-5));
        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.0, 1e-5));
        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.25, 1e-5));
        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.0, 1e-5));
    }

    SECTION("Test Freq, Mul, Add") {
        oscillator.freq.set_value(0.4f);
        oscillator.mul.set_value(2.0f);
        oscillator.add.set_value(1.0f);
        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(1.0, 1e-5));
        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(1.8, 1e-5));
        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(2.6, 1e-5));
        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(1.4, 1e-5));
        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(2.2, 1e-5));
    }

    SECTION("Test Variable Freq, Add, Mul") {
        oscillator.freq.set_value(0.4f);
        oscillator.mul.set_value(2.0f);
        oscillator.add.set_value(1.0f);
        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(1.0, 1e-5)); // x = 0
        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(1.8, 1e-5)); // x = 0.4


        oscillator.freq.set_value(0.2f);
        oscillator.mul.set_value(0.5f);
        oscillator.add.set_value(2.0f);
        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(2.3, 1e-5)); // x = 0.6

        oscillator.freq.set_value(0.6f);
        oscillator.mul.set_value(1.5f);
        oscillator.add.set_value(0.5f);
        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.8, 1e-5)); // x = 0.2
    }
}
