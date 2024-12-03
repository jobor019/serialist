#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/policies/policies.h"
#include "core/algo/temporal/phase_accumulator.h"
#include "core/algo/temporal/time_point.h"
#include "core/generatives/oscillator.h"

#include <thread>
#include <cmath>

#include "node_runner.h"
#include "matchers/v11.h"

using namespace serialist;

static inline PhaseAccumulator initialize_phase_accumulator(double step_size, double phase, PaMode mode) {
    PhaseAccumulator p;
    p.set_mode(mode);
    p.set_step_size(step_size);
    p.set_offset(DomainDuration{phase, DomainType::ticks});
    return p;
}


TEST_CASE("m_phasor stepped", "[oscillator][phasor]") {

    SECTION("unit m_phasor") {
        double step = 0.1;
        auto p = initialize_phase_accumulator(step, 0.0, PaMode::triggered);

        double x;
        TimePoint t; // value irrelevant when stepped

        for (int i = 0; i < 100; ++i) {
            x = p.process(t, true);
            REQUIRE_THAT(x, Catch::Matchers::WithinAbs(std::fmod(step * i, 1.0), 1e-8));
            REQUIRE(x < 1.0);
        }
    }

    SECTION("Negative step size") {
        double step = -0.1;
        auto p = initialize_phase_accumulator(step, 0.0, PaMode::triggered);

        double x;
        double y = 0;
        TimePoint t; // value irrelevant when stepped
        for (int i = 0; i < 100; ++i) {
            x = p.process(t, true);
            if (y < 0) {
                y += 1.0;
            }
            REQUIRE_THAT(x, Catch::Matchers::WithinAbs(y, 1e-8));
            REQUIRE(x < 1.0);
            REQUIRE(x >= 0);
            y += step;
        }
    }

    SECTION("Variable step size") {
        auto step = 0.0;
        auto p = initialize_phase_accumulator(step, 0.0, PaMode::triggered);
        TimePoint t; // value irrelevant when stepped

        REQUIRE_THAT(p.process(t, true), Catch::Matchers::WithinAbs(0.0, 1e-8));
        p.set_step_size(0.2);
        REQUIRE_THAT(p.process(t, true), Catch::Matchers::WithinAbs(0.2, 1e-8));
        p.set_step_size(0.8);
        REQUIRE_THAT(p.process(t, true), Catch::Matchers::WithinAbs(0.0, 1e-8));
    }
}


// ==============================================================================================

TEST_CASE("Oscillator ctor") {
    auto oscillator = OscillatorWrapper();
}


TEST_CASE("Unit Oscillator") {
    using namespace serialist::test;

    auto config = TestConfig().with_step_rounding(StepRounding::exact_stop_before);
    auto oscillator = OscillatorWrapper();
    auto o = &oscillator.oscillator;
    //
    NodeRunner runner(o, config);
    auto r = runner.step_until(DomainTimePoint::ticks(1.0 - EPSILON));

    REQUIRE_THAT(r, v11h::strictly_increasing<Facet>());
    REQUIRE_THAT(r, v11::approx_eqf(1.0 - config.step_size.get_value()));

    r = runner.step_n(1);
    REQUIRE_THAT(r, v11::approx_eqf(0.0));


    // REQUIRE(r.output().voices == Voices<Facet>::singular(Facet(1.0)));
    // r = runner.step_until(DomainTimePoint::ticks(1.0 + 1e-8));
    // REQUIRE(r.output().voices == Voices<Facet>::singular(Facet(0.0)));
}


//TEST_CASE("Unity OLD_PhaseAccumulator") {
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
//TEST_CASE("Scheduled Unity OLD_PhaseAccumulator") {
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
//TEST_CASE("Unity OLD_PhaseAccumulator Drifting") {
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
//TEST_CASE("OLD_PhaseAccumulator Oscillator") {
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

//TEST_CASE("Triangle Oscillator") {
//    OscillatorWrapper oscillator;
//    oscillator.type.set_values(Voices<Oscillator::Type>::singular(Oscillator::Type::tri));
//    oscillator.trigger.set_values(Voices<Trigger>::singular(Trigger::pulse_on()));
//    for (std::size_t i = 0; i < 10; ++i) {
//        oscillator.oscillator.update_time(TimePoint());
//        oscillator.oscillator.process().print();
//    }
//}