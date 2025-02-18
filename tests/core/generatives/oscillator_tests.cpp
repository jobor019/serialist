#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/policies/policies.h"
#include "core/generatives/oscillator.h"


#include "node_runner.h"
#include "generators.h"
#include "matchers/m11.h"

using namespace serialist;
using namespace serialist::test;

/**
*  ==============================================================================================
*  Oscillator Testing Guidelines:
*  ==============================================================================================
*
*  In most cases where we need to know that a cycle starts / ends at a particular time, the best solution is to
*  `step_while(c11::strictly_increasingf/c11:strictly_decreasingf)` and check that this time matches our expectation,
*  rather than to step until the expected time and check if the value is 0.0.
*
*  The reason for this is (obviously) rounding errors. While `step_until(t, Anchor::before)` ensures that
*  current_time < t, we cannot be sure that `f(current_time) < f(t)`.
*
*  In case of the Oscillator, if we for example have period = 2.0 and we step until t < 2.0, we can be sure that
*  t < 2.0, but we cannot be sure that (t / p) < 1.0 (where 1.0 is the oscillator's internal modulo condition).
*
*  Still, if it's strictly necessary to test a given time, we can use `c11::circular_eqf()` to check if a value
*  is approximately equal to 0.0 modulo 1.0.
*/

// ==============================================================================================
// R1.: Periodic with Grid Alignment (TL)
// ==============================================================================================

TEST_CASE("Oscillator (TL): Period and offset control duration and start time of cycle (R1.1.1)", "[oscillator]") {
    auto w = OscillatorWrapper();

    w.mode.set_value(PaMode::transport_locked);

    auto step_size = GENERATE(0.001, 0.01);


    auto [t0, period, offset, cycle_start, cycle_end, next_cycle_end, value_per_tick] = GENERATE(
        table<double, double, double, double, double, double, double>( {
            { 0.0, 1.0, 0.0, 0.0, 1.0, 2.0, 1.0},
            { 0.0, 2.0, 0.0, 0.0, 2.0, 4.0, 0.5},
            { 0.0, 0.1, 0.0, 0.0, 0.1, 0.2, 10.0},
            { 0.0, 10.0, 0.0, 0.0, 10.0, 20.0, 0.1},
            { 0.0, 1.0, 0.5, 0.5, 1.5, 2.5, 1.0},
            { 0.0, 2.0, 0.5, 0.5, 2.5, 4.5, 0.5},
            { 0.0, 2.0, 0.1, 0.1, 2.1, 4.1, 0.5},
            { 0.0, 2.0, 1.9, 1.9, 3.9, 5.9, 0.5},
            {10.0, 2.0, 1.9, 11.9, 13.9, 15.9, 0.5},
            })
    );

    // Precision: how close to 1.0 the value is at the end of the cycle given the current step size
    auto value_epsilon = step_size * value_per_tick + EPSILON;

    CAPTURE(t0, period, offset, cycle_start, cycle_end, next_cycle_end, value_epsilon);

    w.period.set_values(period);
    w.offset.set_values(offset);

    NodeRunner runner{&w.oscillator, TestConfig().with_step_size(DomainDuration::ticks(step_size)), TimePoint(t0)};

    // Step until first value in first cycle
    if (offset > 0.0) {
        auto r = runner.step_until(DomainTimePoint::ticks(cycle_start), Anchor::after);
        REQUIRE_THAT(r, m11::approx_eqf(0.0, value_epsilon));
    }

    // Step an entire cycle + one more value (operationalized by the strictly_increasingf condition)
    auto r = runner.step_while(c11::strictly_increasingf());
    REQUIRE_THAT(r, m11::in_rangef(0.0, 1.0, MatchType::all));

    auto [complete_cycle, first_value_of_new_cycle] = r.unpack();
    REQUIRE_THAT(complete_cycle, m11::approx_eqf(1.0, value_epsilon, MatchType::last));
    REQUIRE_THAT(first_value_of_new_cycle, m11::approx_eqf(0.0, value_epsilon));

    // Check that the new cycle starts at exactly at the expected time
    REQUIRE_THAT(first_value_of_new_cycle.time()
                 , TimePointMatcher().with(DomainType::ticks, cycle_end).with_epsilon(step_size));

    r = runner.step_while(c11::strictly_increasingf());
    REQUIRE_THAT(r, m11::in_rangef(0.0, 1.0, MatchType::all));
    REQUIRE_THAT(r.history_subset(), m11::approx_eqf(1.0, value_epsilon, MatchType::last));
    REQUIRE_THAT(r.last_subset().time()
                 , TimePointMatcher().with(DomainType::ticks, next_cycle_end).with_epsilon(step_size));
}


TEST_CASE("Oscillator (TL): Negative period yields phase-inverted cycles (R1.1.2a)", "[oscillator]") {
    auto w = OscillatorWrapper();
    auto step_size = 0.01;

    auto [period_value, value_per_tick] = GENERATE(
        table<double, double>( {
            {-1.0, 1.0},
            {-2.0, 0.5},
            {-0.5, 2.0},
            {-0.1, 10.0},
            {-10.0, 0.1},
            }));
    // Precision: how close to 1.0 the value is at the end of the cycle given the current step size
    auto value_epsilon = step_size * value_per_tick + EPSILON;


    CAPTURE(period_value, value_epsilon);

    w.mode.set_value(PaMode::transport_locked);
    w.period.set_values(period_value);

    NodeRunner runner(&w.oscillator, TestConfig().with_step_size(DomainDuration::ticks(step_size)));

    // Step one full cycle and a single value into the next cycle
    auto r = runner.step_while(c11::strictly_decreasingf());
    REQUIRE_THAT(r, m11::in_rangef(0.0, 1.0, MatchType::all));

    auto [complete_cycle, first_value_of_next_cycle] = r.unpack();

    // Ensure that last value is the start of a new cycle and the previous value the end of the previous cycle
    REQUIRE_THAT(complete_cycle.first_subset(), m11::approx_eqf(1.0, value_epsilon, MatchType::last));
    REQUIRE_THAT(complete_cycle, m11::approx_eqf(0.0, value_epsilon, MatchType::last));
    REQUIRE_THAT(first_value_of_next_cycle, m11::approx_eqf(1.0, value_epsilon));

    // Ensure that the elapsed duration corresponding to one cycle is equivalent to the period
    REQUIRE_THAT(first_value_of_next_cycle.time(), TimePointMatcher(std::abs(period_value)).with_epsilon(step_size));
}


TEST_CASE("Oscillator (TL): Zero period yields constant value 0 (R1.1.2b)", "[oscillator]") {
    auto w = OscillatorWrapper();

    auto offset = GENERATE(0.0, 0.1, 1.0, 100.0);

    w.mode.set_value(PaMode::transport_locked);
    w.period.set_values(0.0);
    w.offset.set_values(offset);

    NodeRunner runner(&w.oscillator);

    auto r = runner.step_n(1000);
    REQUIRE_THAT(r, m11::eqf(0.0, MatchType::all));
}


TEST_CASE("Oscillator (TL): Offset range is mapped onto [0.0, abs(period)), (R1.1.2c)", "[oscillator]") {
    auto w = OscillatorWrapper();
    w.mode.set_value(PaMode::transport_locked);

    NodeRunner runner(&w.oscillator);

    auto value_epsilon = TestConfig::DEFAULT_STEP_SIZE.get_value() + EPSILON;

    SECTION("Offset == abs(Period) or Offset % abs(Period) = 0.0") {
        auto [period, offset, expected_initial_value] = GENERATE(
            table<double, double, double>({
                {1.0, 1.0, 0.0},
                {2.0, 2.0, 0.0},
                {0.5, 0.5, 0.0},
                {0.1, 1.0, 0.0},
                {-1.0, 1.0, 1.0}, // expected value is technically std::nextafter(1.0, 0.0)
                })
        );
        CAPTURE(period, offset, expected_initial_value);

        w.period.set_values(period);
        w.offset.set_values(offset);

        auto r = runner.step();
        REQUIRE_THAT(r, m11::eqf(expected_initial_value, MatchType::last));
    }

    SECTION("Offset > abs(Period)") {
        auto [period, offset, expected_cycle_start_time, expected_cycle_initial_value] = GENERATE(
            table<double, double, double, double>({
                // offset > period
                {1.0, 1.1, 0.1, 0.0},
                {2.0, 5.1, 1.1, 0.0},
                {-1.0, 1.1, 0.1, 1.0}
                })
        );
        CAPTURE(period, offset, expected_cycle_start_time, expected_cycle_initial_value);

        w.period.set_values(period);
        w.offset.set_values(offset);

        auto r = runner.step_until(DomainTimePoint::ticks(expected_cycle_start_time), Anchor::after);
        REQUIRE_THAT(r, m11::approx_eqf(expected_cycle_initial_value, value_epsilon, MatchType::last ));
    }
}


TEST_CASE("Oscillator (TL): Edge cases do not impact output (R1.1.2d, R1.3)", "[oscillator]") {
    auto w = OscillatorWrapper();
    w.mode.set_value(PaMode::transport_locked);
    NodeRunner runner(&w.oscillator);

    auto t1 = DomainTimePoint::ticks(5.0);
    auto [period, expected_value_at_t1, value_per_tick] = GENERATE(
        table<double, double, double>({
            {1.0, 0.0, 1.0},         // (5 % 1)/1 = 0
            {2.0, 0.5, 0.5},         // (5 % 2)/2 = 0.5
            {4.0, 0.25, 0.25},       // (5 % 4)/4 = 0.25
            {6.0, 5.0/6.0, 1.0/6.0}, // (5 % 6)/6 = 0.83333333ö
            })
    );
    auto value_epsilon = runner.get_step_size().get_value() * value_per_tick + EPSILON;
    CAPTURE(period, expected_value_at_t1, value_epsilon);

    w.period.set_values(period);

    SECTION("Mode Switching") {
        auto mode = GENERATE(PaMode::triggered, PaMode::free_periodic);
        w.mode.set_value(mode);

        auto r = runner.step_until(t1, Anchor::before);
        REQUIRE(r.is_successful());

        w.mode.set_value(PaMode::transport_locked);
        r = runner.step();
        REQUIRE_THAT(r, m11::approx_eqf(expected_value_at_t1, value_epsilon));
    }

    SECTION("Enable/Disable") {
        w.enabled.set_values(false);

        auto r = runner.step_until(t1, Anchor::before);
        REQUIRE(r.is_successful());

        w.enabled.set_values(true);
        r = runner.step();
        REQUIRE_THAT(r, m11::approx_eqf(expected_value_at_t1, value_epsilon));
    }

    SECTION("Reset") {
        auto r = runner.step_until(t1, Anchor::before);
        REQUIRE(r.is_successful());

        w.trigger.set_values(Trigger::pulse_on());
        r = runner.step();
        REQUIRE_THAT(r, m11::approx_eqf(expected_value_at_t1, value_epsilon));
    }

    SECTION("Step Size") {
        auto step_size = GENERATE(0.1, 0.2, 0.3, 0.7, -0.1);
        w.step_size.set_values(step_size);

        auto r = runner.step_until(t1, Anchor::before);
        REQUIRE(r.is_successful());

        r = runner.step();
        REQUIRE_THAT(r, m11::approx_eqf(expected_value_at_t1, value_epsilon));
    }

    SECTION("Value Changes") {
        auto initial_period_value = GENERATE(0.1, 3.4, -0.5);
        auto initial_period_type = GENERATE(DomainType::ticks, DomainType::beats);
        auto initial_offset_value = GENERATE(0.0, 0.2);
        auto initial_offset_type = GENERATE(DomainType::ticks, DomainType::bars);

        w.period.set_values(initial_period_value);
        w.period_type.set_value(initial_period_type);
        w.offset.set_values(initial_offset_value);
        w.offset_type.set_value(initial_offset_type);

        auto r = runner.step_until(t1, Anchor::before);
        REQUIRE(r.is_successful());

        w.period.set_values(period);
        w.period_type.set_value(DomainType::ticks);
        w.offset.set_values(0.0);
        w.offset_type.set_value(DomainType::ticks);

        r = runner.step();
        REQUIRE_THAT(r, m11::approx_eqf(expected_value_at_t1, value_epsilon));
    }
}


TEST_CASE("Oscillator (TL): Two oscillators with same config produce the same output", "[oscillator]") {
    auto w1 = OscillatorWrapper();
    auto w2 = OscillatorWrapper();

    auto period = GENERATE(1.0, 2.0, 3.0, 0.4);
    auto runner2_start_time = GENERATE(0.0, 0.2, 0.5, 0.6);
    auto end_time = DomainTimePoint::ticks(1.0);

    w1.mode.set_value(PaMode::transport_locked);
    w1.period.set_values(period);
    w2.mode.set_value(PaMode::transport_locked);
    w2.period.set_values(period);

    NodeRunner runner1{&w1.oscillator};
    NodeRunner runner2{&w2.oscillator, TimePoint{runner2_start_time}};

    auto r1 = runner1.step_until(end_time, Anchor::before);
    auto r2 = runner2.step_until(end_time, Anchor::before);

    REQUIRE(r1.v11() == r2.v11());
}


TEST_CASE("Oscillator (TL): Period and offset type controls time domain type of cycle (R1.2.1)", "[oscillator]") {
    // Note: All of this could technically be tested in R1.1.1 as most of it is pure duplication of that test.
    //       Regardless, this is implemented as a separate test to more clearly separate the two requirements and
    //       to avoid building tests that are testing too many things at once.

    auto w = OscillatorWrapper();

    // type = ticks is already tested in earlier tests. This test only tests period_type == offset_type.
    auto type = GENERATE(DomainType::beats, DomainType::bars);

    auto meter = GENERATE(Meter{4, 4}, Meter{5, 8}, Meter{3, 2});

    w.period_type.set_value(type);
    w.offset_type.set_value(type);

    auto [period, offset, cycle_start, cycle_end, value_per_tick] = GENERATE(
        table<double, double, double, double, double>( {
            {1.0, 0.0, 0.0, 1.0, 1.0},
            {2.0, 0.0, 0.0, 2.0, 0.5},
            {0.1, 0.0, 0.0, 0.1, 10.0},
            {10.0, 0.0, 0.0, 10.0, 0.1},
            {1.0, 0.5, 0.5, 1.5, 1.0},
            {2.0, 0.5, 0.5, 2.5, 0.5},
            {2.0, 0.1, 0.1, 2.1, 0.5},
            {2.0, 1.9, 1.9, 3.9, 0.5},
            {2.0, 1.9, 11.9, 13.9, 0.5},
            })
    );

    auto step_size = 0.01;
    auto value_epsilon = step_size * value_per_tick + EPSILON;

    CAPTURE(type, period, offset, cycle_start, cycle_end, value_epsilon, meter);

    w.period.set_values(period);
    w.offset.set_values(offset);

    NodeRunner runner{&w.oscillator
                      , TestConfig().with_step_size(DomainDuration{step_size, type})
                      , TimePoint::zero().with_meter(meter)
    };

    // Step until first value in first cycle
    if (offset > 0.0) {
        auto r = runner.step_until(DomainTimePoint{cycle_start, type}, Anchor::after);
        REQUIRE_THAT(r, m11::approx_eqf(0.0, value_epsilon));
    }

    // Step an entire cycle + one more value (operationalized by the strictly_increasingf condition)
    auto r = runner.step_while(c11::strictly_increasingf());
    REQUIRE_THAT(r, m11::in_rangef(0.0, 1.0, MatchType::all));

    auto [complete_cycle, first_value_of_new_cycle] = r.unpack();
    REQUIRE_THAT(complete_cycle, m11::approx_eqf(1.0, value_epsilon, MatchType::last));
    REQUIRE_THAT(first_value_of_new_cycle, m11::approx_eqf(0.0, value_epsilon));

    // Check that the new cycle starts at exactly at the expected time
    REQUIRE_THAT(first_value_of_new_cycle.time()
                 , TimePointMatcher().with(type, cycle_end).with_epsilon(step_size));
}


TEST_CASE("Oscillator (TL): Mixed types are treated as ticks when one type is ticks (R1.2.2a)", "[oscillator]") {
    REQUIRE(false); // TODO: This test is incomplete and incorrect

    auto w = OscillatorWrapper();
    w.mode.set_value(PaMode::transport_locked);

    auto step_size = 0.01;

    auto t1 = DomainTimePoint::ticks(1.0);
    auto [period, offset, meter, expected_value_at_t1, value_per_tick] = GENERATE(
        table<DomainDuration, DomainDuration, Meter, double, double>({
            // 4/4 meter, i.e. 1.0 ticks = 1.0 beats = 0.25 bars
            {DomainDuration::ticks(1.0), DomainDuration::beats(0.5), Meter{4, 4}, 0.5, 1.0},
            {DomainDuration::ticks(2.0), DomainDuration::beats(0.25), Meter{4, 4}, 0.5, 1.0},
            {DomainDuration::ticks(1.0), DomainDuration::bars(0.125), Meter{4, 4}, 0.5, 1.0},

            // // 5/8 meter, i.e. 1.0 ticks = 2.0 beats = 0.4 bars // TODO
            //
            // // 4/2 meter, i.e. 1.0 ticks = 0.5 beats = 0.125 bars // TODO
            //
            // // offset (converted to ticks) > period // TODO
            {DomainDuration::ticks(1.0), DomainDuration::bars(0.5), Meter{4, 4}, 0.0, 1.0}
            })
    );
    auto value_epsilon = step_size * value_per_tick + EPSILON;
    CAPTURE(period, offset, meter, expected_value_at_t1, value_epsilon);

    w.period.set_values(period.get_value());
    w.period_type.set_value(period.get_type());
    w.offset.set_values(offset.get_value());
    w.offset_type.set_value(offset.get_type());

    NodeRunner runner{&w.oscillator
                      , TestConfig().with_step_size(DomainDuration::ticks(step_size))
                      , TimePoint::zero().with_meter(meter)
    };

    auto r = runner.step_until(t1, Anchor::after);
    REQUIRE_THAT(r, m11::approx_eqf(expected_value_at_t1, value_epsilon));
}



// ==============================================================================================
// = R2. Periodic without Grid Alignment (FP) =
// ==============================================================================================

TEST_CASE("Oscillator (FP): Period and offset control duration and start value of cycle (R2.1.1a)", "[oscillator]") {
    auto w = OscillatorWrapper();

    w.mode.set_value(PaMode::free_periodic);

    auto step_size = GENERATE(0.001, 0.01);
    auto period = GENERATE(0.5, 1.0, 2.0, 10.0);
    auto offset = GENERATE(0.1, 0.2, 0.5, 0.8);

    w.period.set_values(period);
    w.offset.set_values(offset);

    CAPTURE(period, offset, step_size);

    NodeRunner runner{&w.oscillator, TestConfig().with_step_size(DomainDuration::ticks(step_size))};

    // First value should be offset
    auto r = runner.step();
    REQUIRE_THAT(r, m11::eqf(offset));

    // Step until end of current cycle
    r = runner.step_until(c11::eqf(0.0));
    REQUIRE_THAT(r.history_subset(), m11::strictly_increasingf());

    // Step until one full period has elapsed from the initial value
    r = runner.step_until(DomainTimePoint::ticks(period), Anchor::after);
    REQUIRE_THAT(r, m11::strictly_increasingf());
    REQUIRE_THAT(r, m11::approx_eqf(offset, step_size + EPSILON));
}

TEST_CASE("Oscillator (FP): Output is continuous when period changes (R2.1.1b)", "[oscillator]") {
    // TODO
}


TEST_CASE("Oscillator (FP): Zero period yields constant previous/offset value  (R2.1.2b)", "[oscillator]") {
    auto w = OscillatorWrapper();

    // auto offset = GENERATE(0.0, 0.1, 1.0, 100.0);
    auto offset = GENERATE(0.0);

    w.mode.set_value(PaMode::free_periodic);
    w.offset.set_values(offset);

    SECTION("No previous value: yield offset value") {
        w.period.set_values(0.0);
        NodeRunner runner(&w.oscillator);
        auto r = runner.step_n(1000);
        REQUIRE_THAT(r, m11::eqf(offset, MatchType::all));
    }

    SECTION("Previous value exists: yield previous value") {
        w.period.set_values(1.0);
        NodeRunner runner(&w.oscillator);

        double target_value = 0.5;

        auto r = runner.step_until(c11::eqf(target_value));
        REQUIRE_THAT(r, m11::eqf(target_value, MatchType::last));

        w.period.set_values(0.0);
        r = runner.step_n(100);
        REQUIRE_THAT(r, m11::eqf(target_value, MatchType::all));
    }
}




// ==============================================================================================
// = LEGACY TESTS =
// ==============================================================================================


TEST_CASE("Oscillator: ctor", "[oscillator]") {
    auto w = OscillatorWrapper();
}


TEST_CASE("Oscillator: enable", "[oscillator]") {
    auto w = OscillatorWrapper();

    auto& oscillator = w.oscillator;
    auto& enabled = w.enabled;

    w.mode.set_value(PaMode::transport_locked); // unit phase oscillator

    NodeRunner runner{&oscillator};

    enabled.set_values(false); // Disable output

    auto r = runner.step_until(DomainTimePoint::ticks(0.5), Anchor::after);
    REQUIRE_THAT(r, m11::emptyf(MatchType::all));

    // Enabling mid-phase: should generate values from 0.5 to 1.0
    enabled.set_values(true);
    r = runner.step_until(DomainTimePoint::ticks(1.0), Anchor::before);
    REQUIRE_THAT(r, m11::in_rangef(0.5, 1.0, MatchType::all));
    REQUIRE_THAT(r, m11::strictly_increasingf(MatchType::all));

    enabled.set_values(false);
    r = runner.step_until(DomainTimePoint::ticks(2.0), Anchor::before);
    REQUIRE_THAT(r, m11::emptyf(MatchType::all));
}


TEST_CASE("Oscillator (TL/FP): Period controls the duration of a cycle (R1.1.1, R1.2.1)", "[oscillator]") {
    auto w = OscillatorWrapper();
    auto step_size = 0.01;
    auto meter = GENERATE(Meter{4, 4}, Meter{2, 8}, Meter{3, 2});

    // With offset = 0 and no changes in period, we expect the same behaviour for TL and FP
    auto mode = GENERATE(PaMode::transport_locked, PaMode::free_periodic);

    auto period_type = GENERATE(DomainType::ticks, DomainType::beats, DomainType::bars);

    // The precision of the oscillator's output (i.e. how close to 1.0 it is at the end of the cycle)
    // depends on the step size in relation to the period. Formula is:
    // epsilon = step_size / period + EPSILON
    auto [period_value, value_epsilon] = GENERATE(
        table<double, double>( {
            {1.0, 0.01},
            {2.0, 0.005},
            {0.5, 0.02},
            {0.1, 0.1},
            {10.0, 0.001},
            })
    );
    value_epsilon = meter.ticks2domain(value_epsilon, period_type) + EPSILON;

    CAPTURE(mode, period_value, period_type, value_epsilon, meter);

    w.mode.set_value(mode);
    w.period.set_values(period_value);
    w.period_type.set_value(period_type);

    NodeRunner runner(&w.oscillator
                      , TestConfig().with_step_size(DomainDuration::ticks(step_size))
                      , TimePoint{}.with_meter(meter)
    );

    // Step one full cycle and a single value into the next cycle
    auto r = runner.step_while(c11::strictly_increasingf());
    REQUIRE(r.is_successful());
    REQUIRE_THAT(r, m11::in_rangef(0.0, 1.0, MatchType::all));

    auto [hist, last] = r.unpack();

    // Ensure that last value is the start of a new cycle and the previous value the end of the previous cycle
    REQUIRE_THAT(hist, m11::approx_eqf(1.0, value_epsilon, MatchType::last));
    REQUIRE_THAT(last, m11::approx_eqf(0.0, value_epsilon));

    // Ensure that the elapsed duration corresponding to one cycle is equivalent to the period
    auto time_epsilon = meter.ticks2domain(step_size, period_type);
    REQUIRE_THAT(last.time(), TimePointMatcher().with(period_type, period_value).with_epsilon(time_epsilon));
}


TEST_CASE("Oscillator (TL): Offset controls the temporal offset (R1.1.3)", "[oscillator]") {
    auto w = OscillatorWrapper();
    auto step_size = 0.01;

    auto meter = GENERATE(Meter{4, 4}, Meter{2, 8}, Meter{3, 2});
    auto offset = GENERATE(0.1, 0.427, 0.5, 0.72, 0.99);
    auto domain_type = GENERATE(DomainType::ticks, DomainType::beats, DomainType::bars);

    CAPTURE(offset, domain_type, meter);

    w.mode.set_value(PaMode::transport_locked);
    w.period_type.set_value(domain_type); // Unit domain period (1 bar / 1 beat / 1 tick)
    w.offset.set_values(offset);
    w.offset_type.set_value(domain_type);

    NodeRunner runner{&w.oscillator
                      , TestConfig().with_step_size(DomainDuration::ticks(step_size))
                      , TimePoint{}.with_meter(meter)
    };

    // Step to the start of the next cycle
    auto r = runner.step_while(c11::strictly_increasingf());
    REQUIRE(r.is_successful());
    REQUIRE_THAT(r, m11::in_rangef(0.0, 1.0, MatchType::all));

    auto time_epsilon = meter.ticks2domain(step_size, domain_type);
    REQUIRE_THAT(r.time(), TimePointMatcher().with(domain_type, offset).with_epsilon(time_epsilon));

    // Step to the start of the following cycle
    r = runner.step_while(c11::strictly_increasingf());
    REQUIRE_THAT(r.time(), TimePointMatcher().with(domain_type, offset + 1.0).with_epsilon(time_epsilon));
}


TEST_CASE("Oscillator: Offset controls the value offset in mode FP", "[oscillator]") {
    auto w = OscillatorWrapper();
    auto step_size = 0.01;

    auto meter = GENERATE(Meter{4, 4}, Meter{2, 8}, Meter{3, 2});
    auto offset = GENERATE(0.1, 0.427, 0.5, 0.72, 0.99);
    auto domain_type = GENERATE(DomainType::ticks, DomainType::beats, DomainType::bars);

    CAPTURE(offset, domain_type, meter);

    w.mode.set_value(PaMode::free_periodic);
    w.period_type.set_value(domain_type);
    w.offset.set_values(offset);
    w.offset_type.set_value(domain_type);

    NodeRunner runner{
        &w.oscillator
        , TestConfig().with_step_size(DomainDuration::ticks(step_size))
        , TimePoint{}.with_meter(meter)
    };

    auto r = runner.step();
    REQUIRE(r.is_successful());

    // Initial value should be exactly the given offset
    REQUIRE_THAT(r, m11::eqf(offset));

    // Step a full period
    r = runner.step_until(DomainTimePoint(1.0, domain_type), Anchor::after);
    auto value_epsilon = meter.ticks2domain(step_size, domain_type);
    REQUIRE_THAT(r, m11::approx_eqf(offset, value_epsilon));
}


TEST_CASE("Oscillator: Offset range is determined by period in mode TL", "[oscillator]") {
    auto w = OscillatorWrapper();
    auto step_size = 0.01;
    auto epsilon = step_size + EPSILON;

    NodeRunner runner{&w.oscillator, TestConfig().with_step_size(DomainDuration::ticks(step_size))};

    auto [period, offset, expected, following_cycle] = GENERATE(
        table<double, double, double, double>( {
            {2.0, 1.3, 1.3, 3.3},
            {2.0, 2.3, 0.3, 2.3},
            {-2.0, 1.3, 1.3, 3.3},
            {-2.0, 2.3, 0.3, 2.3}
            })
    );
    CAPTURE(period, offset);

    w.period.set_values(period);
    w.offset.set_values(offset);

    w.mode.set_value(PaMode::transport_locked);

    // Step to the start of the next cycle
    auto r = runner.step_while(period > 0.0 ? c11::strictly_increasingf() : c11::strictly_decreasingf());
    REQUIRE(r.is_successful());
    REQUIRE_THAT(r.time(), TimePointMatcher(expected).with_epsilon(epsilon));

    // Step to the start of the following cycle
    r = runner.step_while(period > 0.0 ? c11::strictly_increasingf() : c11::strictly_decreasingf());
    REQUIRE_THAT(r.time(), TimePointMatcher(following_cycle).with_epsilon(epsilon));
}


TEST_CASE("Oscillator: Offset range is mapped to [0.0, 1.0] in mode FP", "[oscillator]") {
    auto w = OscillatorWrapper();
    auto step_size = 0.01;
    auto epsilon = step_size + EPSILON;

    NodeRunner runner{&w.oscillator, TestConfig().with_step_size(DomainDuration::ticks(step_size))};

    auto [period, offset, expected, following_cycle] = GENERATE(
        table<double, double, double, double>( {
            {2.0, 1.3, 1.3, 3.3}, // TODO: What value do we actually expect here? 0.3? 1.3/2.0?
            // {2.0, 2.3, 0.3, 2.3},
            // {-2.0, 1.3, 1.3, 3.3},
            // {-2.0, 2.3, 0.3, 2.3}
            })
    );
    CAPTURE(period, offset);

    w.period.set_values(period);
    w.offset.set_values(offset);

    w.mode.set_value(PaMode::free_periodic);

    // TODO Wait.. what do we want here really? At the moment, the value is set to offset/period = 0.65,
    //      but if we have an offset of 1.3 (or 0.3), do we really want this to scale with the period?
    //      This seems like very poor sync behaviour, but are there other use cases where this would be desirable??
    //      CONCLUSION: There doesn't seem to be any reasonable reason for doing this. FIX: modulo instead of scale!!
    auto r = runner.step();
    REQUIRE_THAT(r, m11::eqf(expected));

    // Step a full period
    r = runner.step_until(DomainTimePoint::ticks(period), Anchor::after);
    REQUIRE_THAT(r, m11::approx_eqf(following_cycle, epsilon));
}
