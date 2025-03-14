#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/policies/policies.h"
#include "node_runner.h"
#include "core/generatives/phase_pulsator.h"
#include "generatives/oscillator.h"

#include "generators.h"
#include "matchers/m1m.h"
#include "matchers/m11.h"

using namespace serialist;
using namespace serialist::test;


struct OscillatorPairedPulsator {
    explicit OscillatorPairedPulsator(double initial_legato = 1.0
               , double period = 1.0
               , double offset = 0.0
               , DomainType type = DomainType::ticks) {
        phase_pulsator.legato_amount.set_values(initial_legato);

        oscillator.period.set_values(period);
        oscillator.offset.set_values(offset);
        oscillator.period_type.set_value(type);
        oscillator.offset_type.set_value(type);

        phase_pulsator.pulsator_node.set_cursor(&oscillator.oscillator);

        runner = NodeRunner{&phase_pulsator.pulsator_node};
        runner.add_generative(oscillator.oscillator);

        time_epsilon = runner.get_config().step_size.get_value() + EPSILON;
    }

    OscillatorPairedPulsator& with_period(double period) {
        oscillator.period.set_values(period);
        return *this;
    }

    PhasePulsatorWrapper<> phase_pulsator;
    OscillatorWrapper<> oscillator;
    double time_epsilon;
    NodeRunner<Trigger> runner;
};


TEST_CASE("PhasePulsator: Forward phase triggers new pulse exactly at period (R1.1.1)", "[phase_pulsator]") {
    OscillatorPairedPulsator p;
    auto& runner = p.runner;

    // initial step to phase 0.0 => trigger pulse_on
    auto r = runner.step();
    REQUIRE_THAT(r, m1m::equalst_on());
    REQUIRE_THAT(RunResult<Facet>::dummy(p.oscillator.oscillator.process()), m11::eqf(0.0));
    auto pulse_on_id = *r.pulse_on_id();

    // step one full period (1.0 ticks) => trigger pulse_off matching previous and new pulse_on in same step
    r = runner.step_while(c1m::emptyt());
    REQUIRE_THAT(r, m1m::containst_off(pulse_on_id));
    REQUIRE_THAT(r, m1m::containst_on());
    REQUIRE_THAT(r, m1m::sizet(2));
    REQUIRE_THAT(r, m1m::sortedt());
    REQUIRE_THAT(r.time(), TimePointMatcher(1.0).with_epsilon(p.time_epsilon));
    pulse_on_id = *r.pulse_on_id();

    // step another full period (2.0 ticks) => trigger pulse_off matching previous and new pulse_on in same step
    r = runner.step_while(c1m::emptyt());
    REQUIRE_THAT(r, m1m::containst_off(pulse_on_id));
    REQUIRE_THAT(r, m1m::containst_on());
    REQUIRE_THAT(r, m1m::sizet(2));
    REQUIRE_THAT(r, m1m::sortedt());
    REQUIRE_THAT(r.time(), TimePointMatcher(2.0).with_epsilon(p.time_epsilon));
}

TEST_CASE("PhasePulsator: Backward phase triggers new pulse exactly at period (R1.1.2)", "[phase_pulsator]") {
    OscillatorPairedPulsator p;
    p.with_period(-1.0);
    auto& runner = p.runner;

    // initial Oscillator step to phase ~1.0 => trigger pulse_on
    auto r = runner.step();
    REQUIRE_THAT(r, m1m::equalst_on());
    REQUIRE_THAT(RunResult<Facet>::dummy(p.oscillator.oscillator.process()), m11::eqf(1.0));
    auto pulse_on_id = *r.pulse_on_id();

    // step one full period (1.0 ticks) => trigger pulse_off matching previous and new pulse_on in same step
    r = runner.step_while(c1m::emptyt());
    CAPTURE(p.oscillator.oscillator.process());
    REQUIRE_THAT(r, m1m::containst_off(pulse_on_id));
    REQUIRE_THAT(r, m1m::containst_on());
    REQUIRE_THAT(r, m1m::sizet(2));
    REQUIRE_THAT(r, m1m::sortedt());
    REQUIRE_THAT(r.time(), TimePointMatcher(1.0).with_epsilon(p.time_epsilon));
    pulse_on_id = *r.pulse_on_id();

    // step another full period (2.0 ticks) => trigger pulse_off matching previous and new pulse_on in same step
    r = runner.step_while(c1m::emptyt());
    REQUIRE_THAT(r, m1m::containst_off(pulse_on_id));
    REQUIRE_THAT(r, m1m::containst_on());
    REQUIRE_THAT(r, m1m::sizet(2));
    REQUIRE_THAT(r, m1m::sortedt());
    REQUIRE_THAT(r.time(), TimePointMatcher(2.0).with_epsilon(p.time_epsilon));


}

