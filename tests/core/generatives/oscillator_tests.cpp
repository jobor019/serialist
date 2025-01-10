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


// ==============================================================================================
// TEST SUITE A: Basic Oscillation in Every Mode
// ==============================================================================================

TEST_CASE("Oscillator: Period controls the duration of a cycle in modes TL/FP", "[oscillator]") {
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
            }));
    value_epsilon = meter.ticks2domain(value_epsilon, period_type) + EPSILON;

    w.mode.set_value(mode);
    w.period.set_values(period_value);
    w.period_type.set_value(period_type);

    NodeRunner runner(&w.oscillator
                      , TestConfig().with_step_size(DomainDuration::ticks(step_size))
                      , TimePoint{}.with_meter(meter)
    );

    NodeRunner runner2{
        &w.oscillator
        , TestConfig().with_step_size(DomainDuration::ticks(step_size))
        , TimePoint{}.with_meter(meter)
    };

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


TEST_CASE("Oscillator: Offset controls the temporal offset in mode TL", "[oscillator]") {
    auto w = OscillatorWrapper();
    auto step_size = 0.01;

    auto meter = GENERATE(Meter{4, 4}, Meter{2, 8}, Meter{3, 2});
    auto offset = GENERATE(0.1, 0.427, 0.5, 0.72, 0.99);
    auto domain_type = GENERATE(DomainType::ticks, DomainType::beats, DomainType::bars);

    w.mode.set_value(PaMode::transport_locked);
    w.period_type.set_value(domain_type); // Unit domain period (1 bar / 1 beat / 1 tick)
    w.offset.set_values(offset);
    w.offset_type.set_value(domain_type);

    NodeRunner runner{
        &w.oscillator
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


