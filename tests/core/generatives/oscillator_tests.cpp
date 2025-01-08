#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/policies/policies.h"
#include "core/temporal/phase_accumulator.h"
#include "core/temporal/time_point.h"
#include "core/generatives/oscillator.h"

#include <thread>
#include <cmath>


#include "node_runner.h"
#include "generators.h"
#include "matchers/m11.h"

using namespace serialist;
using namespace serialist::test;


static PhaseAccumulator initialize_phase_accumulator(double step_size, double phase, PaMode mode) {
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

// TEST_CASE("Oscillator: ctor", "[oscillator]") {
//     auto oscillator = OscillatorWrapper();
// }
//
//
// TEST_CASE("Oscillator: enabled", "[oscillator]") {
//     auto oscillator = OscillatorWrapper();
//
//     auto& o = oscillator.oscillator;
//     auto& e = oscillator.enabled;
//
//     oscillator.mode.set_value(PaMode::transport_locked); // unit phase oscillator
//
//     NodeRunner runner(&o);
//
//     // Disable output
//     e.set_values(false);
//
//     auto r = runner.step_until(DomainTimePoint::ticks(0.5), Stop::after);
//
//     REQUIRE_THAT(r, emptyf());
//     REQUIRE_THAT(r, all_emptyf());
//
//     // Enabling mid-phase: should generate (history) values from 0.5 to 1.0
//     e.set_values(true);
//     r = runner.step_until(DomainTimePoint::ticks(1.0), Stop::before);
//     REQUIRE_THAT(r, v11h::allf(v11::gef(0.5)) && v11h::allf(v11::ltf(1.0)));
//
//     e.set_values(false);
//     r = runner.step_until(DomainTimePoint::ticks(2.0), Stop::before);
//     REQUIRE_THAT(r, emptyf());
//     REQUIRE_THAT(r, all_emptyf());
// }
//
//
// TEST_CASE("Oscillator: fixed period/offset oscillation", "[oscillator]") {
//     auto oscillator = OscillatorWrapper();
//     auto& o = oscillator.oscillator;
//
//     auto& mode = oscillator.mode;
//
//     // double period_value = GENERATE(1.0, 2.0, 10.0);
//     double period_value = GENERATE(2.0);
//     // double offset_value = GENERATE(0.1, 0.2, 0.5, 0.75);
//     double offset_value = GENERATE(0.1);
//     double transport_step_size = 0.01;
//     // DomainType domain_type = GENERATE(DomainType::ticks, DomainType::beats, DomainType::bars);
//     DomainType domain_type = GENERATE(DomainType::ticks);
//
//     auto t0 = TimePoint{0.0};
//
//     CAPTURE(period_value, offset_value, transport_step_size, domain_type, t0);
//
//     oscillator.period.set_values(period_value);
//     oscillator.offset.set_values(offset_value);
//     oscillator.period_type.set_value(domain_type);
//     oscillator.offset_type.set_value(domain_type);
//
//     auto config = TestConfig().with_step_size(DomainDuration(transport_step_size));
//
//     NodeRunner runner(&o, config, t0);
//
//     // Each step increments the oscillator by transport_step_size / period_value
//     auto value_epsilon = EPSILON + transport_step_size / period_value;
//     CAPTURE(value_epsilon);
//
//     SECTION("Mode: transport-locked") {
//         mode.set_value(PaMode::transport_locked);
//
//         // In transport-locked mode, the phase offset defines at which time (modulo period) the phase starts/ends
//         auto phase_end = t0
//                     + DomainDuration(period_value, domain_type)
//                     - utils::modulo(t0.get(domain_type) - offset_value, period_value);
//
//         CAPTURE(phase_end);
//
//         // Given starting time, step partial cycle until right before phase end
//         auto r = runner.step_until(phase_end, Stop::before);
//         REQUIRE_THAT(r, v11h::strictly_increasingf());
//         REQUIRE_THAT(r, v11::in_rangef(1.0 - value_epsilon, 1.0));
//         REQUIRE_THAT(r, v11h::allf(v11::in_unit_rangef()));
//
//         r = runner.step();
//         REQUIRE_THAT(r, v11::in_rangef(0.0, value_epsilon));
//
//         // Step an entire cycle
//         phase_end += period_value;
//         r = runner.step_until(phase_end, Stop::before);
//         REQUIRE_THAT(r, v11h::strictly_increasingf());
//         REQUIRE_THAT(r, v11::approx_eqf(1.0, value_epsilon));
//         REQUIRE_THAT(r, v11h::allf(v11::in_unit_rangef()));
//
//         r = runner.step();
//         REQUIRE_THAT(r, v11::approx_eqf(0.0, value_epsilon));
//     }


    // SECTION("Mode: free periodic") {
    //     mode.set_value(PaMode::free_periodic);
    //
    //     auto r = runner.step();
    //     REQUIRE_THAT(r, v11::eqf(offset_value));
    //
    //     auto phase_end = t0 + DomainDuration((1.0 - offset_value) / period_value, domain_type);
    //     CAPTURE(phase_end);
    //     FAIL("TODO");
    // }
// }
