#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/algo/phasor.h"
#include "core/generatives/variable.h"
#include "core/generatives/oscillator.h"
#include "core/param/parameter_policy.h"
#include "core/generatives/unit_pulse.h"

#include <thread>
#include <cmath>




TEST_CASE("m_phasor stepped", "[m_phasor]") {

    SECTION("unity m_phasor") {
        double phase = 0.0;
        double max = 1.0;
        bool stepped = true;

        double gain = 0.1;

        Phasor p{max, 0.0, 0.0};

        double x;

        for (int i = 0; i < 100; ++i) {
            x = p.process(0, gain, phase, stepped);
            REQUIRE_THAT(x, Catch::Matchers::WithinAbs(std::fmod(gain * i, max), 1e-8));
            REQUIRE(x < max);
        }
    }

    SECTION("Non-unity maximum") {
        double phase = 0.0;
        double max = 4.5;
        double gain = 0.001;
        bool stepped = true;
        Phasor p{max, 0.0, 0.0};

        double x;
        for (int i = 0; i < 100; ++i) {
            x = p.process(0, gain, phase, stepped);
            REQUIRE_THAT(x, Catch::Matchers::WithinAbs(std::fmod(gain * i, max), 1e-8));
            REQUIRE(x < max);
        }
    }

    SECTION("Negative step size") {
        double phase = 0.0;
        double gain = -0.1;
        double max = 1.0;
        bool stepped = true;
        Phasor p{max, 0.0, 0.0};


        double x;
        double y = 0;
        for (int i = 0; i < 100; ++i) {
            x = p.process(0, gain, phase, stepped);
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
        double phase = 0.0;
        double max = 1.0;
        bool stepped = true;
        Phasor p{max, 0.0, 0.0};
        REQUIRE_THAT(p.process(0, 0.0, phase, stepped), Catch::Matchers::WithinAbs(0.0, 1e-8));
        REQUIRE_THAT(p.process(0, 0.2, phase, stepped), Catch::Matchers::WithinAbs(0.2, 1e-8));
        REQUIRE_THAT(p.process(0, 0.8, phase, stepped), Catch::Matchers::WithinAbs(0.0, 1e-8));
    }

    SECTION("Variable phase") {
        double phase = 0.5;
        double max = 1.0;
        bool stepped = true;
        double gain = 0.1;
        Phasor p{max, 0.0,  0.0};
        REQUIRE_THAT(p.process(0, gain, phase, stepped), Catch::Matchers::WithinAbs(0.5, 1e-8));
        REQUIRE_THAT(p.process(0, gain, phase, stepped), Catch::Matchers::WithinAbs(0.6, 1e-8));
        phase = 0.2;
        REQUIRE_THAT(p.process(0, gain, phase, stepped), Catch::Matchers::WithinAbs(0.4, 1e-8));
        REQUIRE_THAT(p.process(0, gain, phase, stepped), Catch::Matchers::WithinAbs(0.5, 1e-8));
        phase = 0.9;
        REQUIRE_THAT(p.process(0, gain, phase, stepped), Catch::Matchers::WithinAbs(0.3, 1e-8));
        REQUIRE_THAT(p.process(0, gain, phase, stepped), Catch::Matchers::WithinAbs(0.4, 1e-8));
    }
}


// ==============================================================================================

TEST_CASE("Oscillator Ctor") {
    auto oscillator = OscillatorWrapper();
}


//TEST_CASE("Unity Phasor") {
//    float increment = 0.3f; // stepped increment 0.3f is equivalent to 0.3 cycles per trigger = 0.3 freq
//
//    auto wrapper = OscillatorWrapper();
//    wrapper.type.try_set_value(OscillatorNode::Mode::phasor);
//    wrapper.freq.try_set_value(increment);
//    wrapper.m_stepped.try_set_value(true);
//
//    TimePoint t;
//
//    wrapper.trigger.update_time(t);
//
//
//    wrapper.m_oscillator.update_time(t);
//    REQUIRE_THAT(static_cast<double>(*wrapper.m_oscillator.process().front()), Catch::Matchers::WithinAbs(0.0, 1e-5));
//    wrapper.m_oscillator.update_time(t);
//    REQUIRE_THAT(static_cast<double>(*wrapper.m_oscillator.process().front()), Catch::Matchers::WithinAbs(0.3, 1e-5));
//    wrapper.m_oscillator.update_time(t);
//    REQUIRE_THAT(static_cast<double>(*wrapper.m_oscillator.process().front()), Catch::Matchers::WithinAbs(0.6, 1e-5));
//    wrapper.m_oscillator.update_time(t);
//    REQUIRE_THAT(static_cast<double>(*wrapper.m_oscillator.process().front()), Catch::Matchers::WithinAbs(0.9, 1e-5));
//    wrapper.m_oscillator.update_time(t);
//    REQUIRE_THAT(static_cast<double>(*wrapper.m_oscillator.process().front()), Catch::Matchers::WithinAbs(0.2, 1e-5));
//    wrapper.m_oscillator.update_time(t);
//    REQUIRE_THAT(static_cast<double>(*wrapper.m_oscillator.process().front()), Catch::Matchers::WithinAbs(0.5, 1e-5));
//}
//
//TEST_CASE("Scheduled Unity Phasor") {
//    float freq = 0.25f;
//
//    auto wrapper = OscillatorWrapper();
//    wrapper.type.try_set_value(OscillatorNode::Mode::phasor);
//    wrapper.freq.try_set_value(freq);
//    wrapper.m_stepped.try_set_value(false);
//
//    TimePoint t{0.0};
//
//    wrapper.trigger.update_time(t);
//
//    wrapper.m_oscillator.update_time(t);
//    REQUIRE_THAT(static_cast<double>(*wrapper.m_oscillator.process().front()), Catch::Matchers::WithinAbs(0.0, 1e-5));
//
//    t = TimePoint{1.0};
//    wrapper.m_oscillator.update_time(t);
//    REQUIRE_THAT(static_cast<double>(*wrapper.m_oscillator.process().front()), Catch::Matchers::WithinAbs(0.25, 1e-5));
//
//    t = TimePoint{2.0};
//    wrapper.m_oscillator.update_time(t);
//    REQUIRE_THAT(static_cast<double>(*wrapper.m_oscillator.process().front()), Catch::Matchers::WithinAbs(0.5, 1e-5));
//
//    t = TimePoint{3.0};
//    wrapper.m_oscillator.update_time(t);
//    REQUIRE_THAT(static_cast<double>(*wrapper.m_oscillator.process().front()), Catch::Matchers::WithinAbs(0.75, 1e-5));
//
//    t = TimePoint{4.0};
//    wrapper.m_oscillator.update_time(t);
//    REQUIRE_THAT(static_cast<double>(*wrapper.m_oscillator.process().front()), Catch::Matchers::WithinAbs(0.0, 1e-5));
//
//}

//
//
//TEST_CASE("Unity Phasor Drifting") {
//    auto oscillator = OscillatorWrapper();
//    oscillator.osc_type.try_set_value(Oscillator::Mode::phasor);
//    oscillator.freq.try_set_value(0.1f);
//
//    TimePoint t;
//
//    // TODO: Obvious drifting issues here
//    for (int i = 0; i < 10000; ++i) {
//        auto v = oscillator.oscillator.process(t).at(0);
//        std::cout << i << ": " << v << "\n";
//        REQUIRE_THAT(v, Catch::Matchers::WithinAbs((i % 10) / 10.0f, 1e-5));
//    }
//}
//
//
//TEST_CASE("Sin Oscillator with Add Value") {
//    auto oscillator = OscillatorWrapper();
//    oscillator.osc_type.try_set_value(Oscillator::Mode::sin);
//    oscillator.freq.try_set_value(0.25f);
//    oscillator.add.try_set_value(2.0f);
//
//    TimePoint t;
//
//    REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(2.0, 1e-5));
//    REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(2.5, 1e-5));
//    REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(3.0, 1e-5));
//    REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(2.5, 1e-5));
//    REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(2.0, 1e-5));
//}
//
//
//TEST_CASE("Phasor Oscillator") {
//    OscillatorWrapper oscillator;
//    oscillator.osc_type.try_set_value(Oscillator::Mode::phasor);
//    oscillator.freq.try_set_value(0.25f);
//
//    TimePoint t;
//
//    SECTION("Test Uniform") {
//        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.0, 1e-5));
//        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.25, 1e-5));
//        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.5, 1e-5));
//        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.75, 1e-5));
//        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.0, 1e-5));
//    }
//
//    SECTION("Test Freq and Mul") {
//        oscillator.freq.try_set_value(0.5f);
//        oscillator.mul.try_set_value(0.5f);
//
//        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.0, 1e-5));
//        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.25, 1e-5));
//        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.0, 1e-5));
//        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.25, 1e-5));
//        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.0, 1e-5));
//    }
//
//    SECTION("Test Freq, Mul, Add") {
//        oscillator.freq.try_set_value(0.4f);
//        oscillator.mul.try_set_value(2.0f);
//        oscillator.add.try_set_value(1.0f);
//        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(1.0, 1e-5));
//        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(1.8, 1e-5));
//        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(2.6, 1e-5));
//        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(1.4, 1e-5));
//        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(2.2, 1e-5));
//    }
//
//    SECTION("Test Variable Freq, Add, Mul") {
//        oscillator.freq.try_set_value(0.4f);
//        oscillator.mul.try_set_value(2.0f);
//        oscillator.add.try_set_value(1.0f);
//        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(1.0, 1e-5)); // x = 0
//        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(1.8, 1e-5)); // x = 0.4
//
//
//        oscillator.freq.try_set_value(0.2f);
//        oscillator.mul.try_set_value(0.5f);
//        oscillator.add.try_set_value(2.0f);
//        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(2.3, 1e-5)); // x = 0.6
//
//        oscillator.freq.try_set_value(0.6f);
//        oscillator.mul.try_set_value(1.5f);
//        oscillator.add.try_set_value(0.5f);
//        REQUIRE_THAT(oscillator.oscillator.process(t).at(0), Catch::Matchers::WithinAbs(0.8, 1e-5)); // x = 0.2
//    }
//}

TEST_CASE("Triangle Oscillator") {
    OscillatorWrapper oscillator;
    oscillator.type.set_values(Voices<Oscillator::Type>::singular(Oscillator::Type::tri));
    oscillator.trigger.set_values(Voices<Trigger>::singular(Trigger::pulse_on(std::nullopt)));
    for (std::size_t i = 0; i < 10; ++i) {
        oscillator.oscillator.update_time(TimePoint());
        oscillator.oscillator.process().print();
    }
}