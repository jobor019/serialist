#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
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


double PHASE_MAX = Phase::one().get();

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


TEST_CASE("PhasePulsator: Phase triggers new pulse exactly at period (R1.1.1 & R1.1.2)", "[phase_pulsator]") {
    // Unit phase forward / unit phase backward
    auto [period, expected_phase_at_trigger] = GENERATE(
        table<double, double>({
            {1.0, 0.0},
            {-1.0, 1.0}
        }));

    OscillatorPairedPulsator p(1.0, period);
    auto& runner = p.runner;

    // initial step to phase 0.0 (forward) or 1.0 (backward) => trigger pulse_on
    auto r = runner.step();
    REQUIRE_THAT(r, m1m::equalst_on());
    REQUIRE_THAT(RunResult<Facet>::dummy(p.oscillator.oscillator.process()), m11::eqf(expected_phase_at_trigger));
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

TEST_CASE("PhasePulsator: Threshold crossings in opposite directions (R1.1.3)", "[phase_pulsator]") {

    PhasePulsatorWrapper w;
    auto& cursor = w.cursor;
    NodeRunner runner{&w.pulsator_node};


    SECTION("Forward -> Backward does not trigger pulse") {
        cursor.set_values(0.0);
        auto r = runner.step();
        REQUIRE_THAT(r, m1m::equalst_on());

        cursor.set_values(PHASE_MAX);
        r = runner.step();
        REQUIRE_THAT(r, m1m::emptyt());
    }

    SECTION("Backward -> Forward does not trigger pulse") {
        cursor.set_values(PHASE_MAX);
        auto r = runner.step();
        REQUIRE_THAT(r, m1m::equalst_on());

        cursor.set_values(0.0);
        r = runner.step();
        REQUIRE_THAT(r, m1m::emptyt());
    }

    SECTION("(Forward ->) Backward -> Backward triggers pulse") {
        // Initial step: Generate first pulse on
        cursor.set_values(0.0);
        REQUIRE_THAT(runner.step(), m1m::equalst_on());


        // Step backward past the threshold: We don't expect any output here
        cursor.set_values(PHASE_MAX);
        REQUIRE_THAT(runner.step(), m1m::emptyt());

        // Sanity check for current configuration
        assert(SingleThresholdStrategy::JUMP_DETECTION_THRESHOLD > 0.1);

        // Step backwards to the end of the current period
        // Since we're manually updating the cursor and the NodeRunner currently doesn't support
        // this type of behaviour, we have to do it manually
        for (auto cursor_value = PHASE_MAX; cursor_value > 0.0; cursor_value -= 0.1) {
            cursor.set_values(cursor_value);
            REQUIRE_THAT(runner.step(), m1m::emptyt());
        }

        // Step backwards: Since we've completed one full period backwards and are triggering the threshold
        // in the same direction again, we expect a pulse here
        cursor.set_values(PHASE_MAX);
        REQUIRE_THAT(runner.step(), m1m::containst_on());
    }

    SECTION("(Backward ->) Forward -> Forward triggers pulse") {
        // Initial step: Generate first pulse on
        cursor.set_values(PHASE_MAX);
        REQUIRE_THAT(runner.step(), m1m::equalst_on());

        // Step forward past the threshold: We don't expect any output here
        cursor.set_values(0.0);
        REQUIRE_THAT(runner.step(), m1m::emptyt());

        // See previous SECTION
        assert(SingleThresholdStrategy::JUMP_DETECTION_THRESHOLD > 0.1);
        for (auto cursor_value = 0.0; cursor_value < PHASE_MAX; cursor_value += 0.1) {
            cursor.set_values(cursor_value);
            REQUIRE_THAT(runner.step(), m1m::emptyt());
        }

        // Step forward: Since we've completed one full period forward and are triggering the threshold
        // in the same direction again, we expect a pulse here
        cursor.set_values(0.0);
        REQUIRE_THAT(runner.step(), m1m::containst_on());
    }
}

TEST_CASE("PhasePulsator: Initial cursor position", "[phase_pulsator]") {
    PhasePulsatorWrapper w;
    auto& cursor = w.cursor;
    NodeRunner runner{&w.pulsator_node};

    SECTION("Initial cursor close to 0.0 triggers pulse") {
        cursor.set_values(0.0);
        REQUIRE_THAT(runner.step(), m1m::equalst_on());
    }

    SECTION("Initial cursor close to 1.0 triggers pulse") {
        cursor.set_values(1.0);
        REQUIRE_THAT(runner.step(), m1m::equalst_on());
    }

    SECTION("Initial cursor not close to threshold does not trigger pulse") {
        auto cursor_position = GENERATE(0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9);
        cursor.set_values(cursor_position);
        REQUIRE_THAT(runner.step(), m1m::emptyt());
    }

    SECTION("Initial cursor not close to threshold triggers at next threshold crossing (forward)") {
        // Initial cursor not close to threshold
        cursor.set_values(0.9);
        REQUIRE_THAT(runner.step(), m1m::emptyt());

        cursor.set_values(0.95);
        REQUIRE_THAT(runner.step(), m1m::emptyt());

        cursor.set_values(0.0);
        REQUIRE_THAT(runner.step(), m1m::containst_on());
    }

    SECTION("Initial cursor not close to threshold triggers at next threshold crossing (backward)") {
        // Initial cursor not close to threshold
        cursor.set_values(0.1);
        REQUIRE_THAT(runner.step(), m1m::emptyt());

        cursor.set_values(0.05);
        REQUIRE_THAT(runner.step(), m1m::emptyt());

        cursor.set_values(PHASE_MAX);
        REQUIRE_THAT(runner.step(), m1m::containst_on());
    }


}


