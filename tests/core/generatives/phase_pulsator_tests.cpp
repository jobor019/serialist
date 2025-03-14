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

// ==============================================================================================

const double PHASE_MAX = Phase::one().get();

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


    double get_oscillator_phase() {
        // Note: this will not trigger a new value unless we explicitly call update_time on the oscillator first
        return *oscillator.oscillator.process().first();
    }


    TimePointMatcher time_matcher(double tick) const {
        return TimePointMatcher(tick).with_epsilon(time_epsilon);
    }


    /* Note: Until PhaseAccumulator is changed to use Phase internally,
     *       this matcher may suffer from problematic rounding errors,
     *       where the Oscillator outputs 0.9999999999 but the
     *       PhasePulsator (Phase ctor) converts this to 0.0
     */
    Catch::Matchers::WithinAbsMatcher phase_matcher(double phase) const {
        return Catch::Matchers::WithinAbs(phase, time_epsilon);
    }


    PhasePulsatorWrapper<> phase_pulsator;
    OscillatorWrapper<> oscillator;
    double time_epsilon;
    NodeRunner<Trigger> runner;
};

/**
*  ==============================================================================================
*  PhasePulsator Testing Guidelines:
*  ==============================================================================================
*
*  The same issues outlined in Oscillator's test suite (oscillator_tests.cpp) apply here.
*  That is, in regard to rounding errors:
*
*  While `step_until(t, Anchor::before)` ensures that current_time < t, we cannot be sure that `f(current_time) < f(t)`.
*
*  This is highly problematic in all tests below that use the OscillatorPairedPulsator. For example, we cannot be sure
*  that an Oscillator with period = 1.0 will yield phase 0.0 at time = 3.0, even if it yields it at time = 1.0 and 2.0.
*
*  Also, using phase
*
*  // TODO: Note that many tests below still apply this strategy.
*           This needs to be handled at a later stage for test correctness.
*/

// ==============================================================================================
// R1.1: Single Threshold Scenario :: Pulse On Position
// ==============================================================================================


TEST_CASE("PhasePulsator: Phase triggers new pulse exactly at period (R1.1.1 & R1.1.2)", "[phase_pulsator]") {
    // Unit phase forward / unit phase backward
    auto [period, expected_phase_at_trigger] = GENERATE(
        table<double, double>({
            {1.0, 0.0},
            {-1.0, 1.0}
            }));
    CAPTURE(period, expected_phase_at_trigger);

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


TEST_CASE("PhasePulsator: Initial cursor position (R1.1.4)", "[phase_pulsator]") {
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
        CAPTURE(cursor_position);
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


TEST_CASE("PhasePulsator: Cursor jumps are treated as initial cursor position (R1.1.5)", "[phase_pulsator]") {
    PhasePulsatorWrapper w;
    auto& cursor = w.cursor;
    NodeRunner runner{&w.pulsator_node};

    // Before jumping: step until middle of phase
    auto cursor_value = 0.0;
    while (cursor_value < 0.5) {
        cursor.set_values(cursor_value);
        cursor_value += 0.1;
    }

    SECTION("Cursor jump to 0.0") {
        // Sanity check in case JUMP_DETECTION_THRESHOLD is changed
        assert(cursor_value > SingleThresholdStrategy::JUMP_DETECTION_THRESHOLD);

        cursor.set_values(0.0);
        REQUIRE_THAT(runner.step(), m1m::containst_on());
    }

    SECTION("Cursor jump to 1.0") {
        // Sanity check in case JUMP_DETECTION_THRESHOLD is changed
        assert(PHASE_MAX - cursor_value > SingleThresholdStrategy::JUMP_DETECTION_THRESHOLD);

        cursor.set_values(PHASE_MAX);
        REQUIRE_THAT(runner.step(), m1m::containst_on());
    }

    SECTION("Cursor jump to between 0.0 and 1.0") {
        auto jump_position = GENERATE(0.1, 0.9);
        CAPTURE(jump_position);

        // Sanity check in case JUMP_DETECTION_THRESHOLD is changed
        assert(std::abs(jump_position - cursor_value) > SingleThresholdStrategy::JUMP_DETECTION_THRESHOLD);

        cursor.set_values(jump_position);
        REQUIRE_THAT(runner.step(), m1m::emptyt());
    }
}


TEST_CASE("PhasePulsator: Constant cursor does not trigger output (R1.1.6)", "[phase_pulsator]") {
    PhasePulsatorWrapper w;
    auto& cursor = w.cursor;
    NodeRunner runner{&w.pulsator_node};

    SECTION("Constant cursor at threshold") {
        auto cursor_position = GENERATE(0.0, PHASE_MAX);
        CAPTURE(cursor_position);
        cursor.set_values(cursor_position);
        REQUIRE_THAT(runner.step(), m1m::equalst_on());

        REQUIRE_THAT(runner.step_n(100), m1m::emptyt(MatchType::all));
    }

    SECTION("Constant cursor between 0.0 and 1.0") {
        cursor.set_values(0.0);
        REQUIRE_THAT(runner.step(), m1m::equalst_on());

        cursor.set_values(0.1);
        REQUIRE_THAT(runner.step_n(100), m1m::emptyt());
    }
}


// ==============================================================================================
// R1.2: Single Threshold Scenario :: Pulse Off (Legato) Position
// ==============================================================================================

TEST_CASE("PhasePulsator: Phase triggers pulse_off exactly at legato value (R1.2.1)", "[phase_pulsator]") {
    OscillatorPairedPulsator p;
    auto& runner = p.runner;
    auto& legato = p.phase_pulsator.legato_amount;
    auto& period = p.oscillator.period;

    // Unit Oscillator backward & forward
    auto period_value = GENERATE(-1.0, 1.0);
    auto expected_pulse_on_phase = period_value > 0.0 ? 0.0 : PHASE_MAX;
    CAPTURE(period_value);
    CAPTURE(expected_pulse_on_phase);

    period.set_values(period_value);

    // SECTION("Legato = 0.0 (or < 0.0) yields matching pulse_on and pulse_off in same time step") {
    //     auto legato_value = GENERATE(0.0, -0.1, -1.0);
    //     CAPTURE(legato_value);
    //     legato.set_values(legato_value);
    //
    //     // Initial step to 0.0 / PHASE_MAX
    //     auto r = runner.step();
    //     REQUIRE_THAT(r, m1m::sizet(2));
    //     REQUIRE_THAT(r, m1m::sortedt());
    //     REQUIRE_THAT(r, m1m::containst_on());
    //     REQUIRE_THAT(r, m1m::containst_off());
    //     REQUIRE(r.pulse_on_id() == r.pulse_off_id());
    //
    //     // One full cycle to 0.0 / PHASE_MAX
    //     r = runner.step_while(c1m::emptyt());
    //     REQUIRE_THAT(p.get_oscillator_phase(), p.phase_matcher(expected_pulse_on_phase));
    //     REQUIRE_THAT(r, m1m::sizet(2));
    //     REQUIRE_THAT(r, m1m::sortedt());
    //     REQUIRE_THAT(r, m1m::containst_on());
    //     REQUIRE_THAT(r, m1m::containst_off());
    //     REQUIRE(r.pulse_on_id() == r.pulse_off_id());
    // }
    //
    // SECTION("Legato in (0.0, 1.0) yields matching pulse_on and pulse_off in the same cycle") {
    //     auto legato_value = GENERATE(0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9);
    //     auto expected_pulse_off_phase = period_value > 0.0 ? legato_value : 1.0 - legato_value;
    //     CAPTURE(legato_value);
    //     CAPTURE(expected_pulse_off_phase);
    //
    //     legato.set_values(legato_value);
    //
    //     // Initial step to 0.0 / PHASE_MAX
    //     auto r = runner.step();
    //     REQUIRE_THAT(r, m1m::equalst_on());
    //     auto pulse_on_id = *r.pulse_on_id();
    //
    //     // Step until legato threshold: ensure pulse_off
    //     r = runner.step_while(c1m::emptyt());
    //     REQUIRE_THAT(p.get_oscillator_phase(), p.phase_matcher(expected_pulse_off_phase));
    //     REQUIRE_THAT(r, m1m::equalst_off(pulse_on_id));
    //
    //     // Step until end of cycle: ensure new_pulse_on without any pulse_off
    //     r = runner.step_while(c1m::emptyt());
    //     REQUIRE_THAT(p.get_oscillator_phase(), p.phase_matcher(expected_pulse_on_phase));
    //     REQUIRE(r.num_steps() > 1);
    //     REQUIRE_THAT(r, m1m::equalst_on());
    // }
    //
    // SECTION("Legato = 1.0 yields matching pulse_off at the start of the next cycle") {
    //     legato.set_values(1.0);
    //
    //     // Initial step to 0.0 / PHASE_MAX: trigger pulse_on
    //     auto r = runner.step();
    //     REQUIRE_THAT(r, m1m::equalst_on());
    //     auto pulse_on_id = *r.pulse_on_id();
    //
    //     // One full cycle to 0.0: ensure matching pulse_off in same cycle as next pulse_on
    //     r = runner.step_while(c1m::emptyt());
    //     REQUIRE_THAT(p.get_oscillator_phase(), p.phase_matcher(expected_pulse_on_phase));
    //     REQUIRE_THAT(r.time(), p.time_matcher(1.0));
    //     REQUIRE_THAT(r, m1m::sizet(2));
    //     REQUIRE_THAT(r, m1m::sortedt());
    //     REQUIRE_THAT(r, m1m::containst_off(pulse_on_id));
    //     REQUIRE_THAT(r, m1m::containst_on());
    // }
    //
    // SECTION("Legato in (0.0, 2.0) yields matching pulse_off in the middle of next cycle") {
    //     auto legato_value = GENERATE(1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9);
    //     auto expected_pulse_off_phase = period_value > 0.0
    //                                         ? legato_value - 1
    //                                         : 2.0 - legato_value;
    //     CAPTURE(legato_value);
    //     CAPTURE(expected_pulse_off_phase);
    //
    //     legato.set_values(legato_value);
    //
    //     // Initial step to 0.0 / PHASE_MAX
    //     auto r = runner.step();
    //     REQUIRE_THAT(r, m1m::equalst_on());
    //     auto first_pulse_on_id = *r.pulse_on_id();
    //
    //     // Step one full cycle without any pulse_off
    //     r = runner.step_while(c1m::emptyt());
    //     REQUIRE_THAT(p.get_oscillator_phase(), p.phase_matcher(expected_pulse_on_phase));
    //     REQUIRE_THAT(r, m1m::equalst_on());
    //     auto second_pulse_on_id = *r.pulse_on_id();
    //
    //     // Step until legato threshold: ensure pulse_off matching the first pulse
    //     r = runner.step_while(c1m::emptyt());
    //     REQUIRE_THAT(p.get_oscillator_phase(), p.phase_matcher(expected_pulse_off_phase));
    //     REQUIRE_THAT(r, m1m::equalst_off(first_pulse_on_id));
    //
    //     // Step until end of cycle: ensure new pulse_on
    //     r = runner.step_while(c1m::emptyt());
    //     REQUIRE_THAT(p.get_oscillator_phase(), p.phase_matcher(expected_pulse_on_phase));
    //     REQUIRE_THAT(r, m1m::equalst_on()); // Don't care about this id
    //
    //     // Step until legato threshold: ensure pulse_off matching the second pulse
    //     r = runner.step_while(c1m::emptyt());
    //     REQUIRE_THAT(p.get_oscillator_phase(), p.phase_matcher(expected_pulse_off_phase));
    //     REQUIRE_THAT(r, m1m::equalst_off(second_pulse_on_id));
    // }

    // This is part of the same requirement as above, but requires a different test setup
    //   since it doesn't have mid-phase legato thresholds
    SECTION("Legato ~2.0 (or >=2.0) yields matching pulse_off at the end of the next cycle") {
        // Legato values greater than 2.0 should yield exactly the same result, as they should be clipped to range
        auto legato_value = GENERATE(std::nextafter(2.0, 0.0), 2.1, 3.0, 10.0);
        CAPTURE(legato_value);
        legato.set_values(legato_value);

        // Initial step to 0.0 / PHASE_MAX
        auto r = runner.step();
        REQUIRE_THAT(r, m1m::equalst_on());
        auto first_pulse_on_id = *r.pulse_on_id();
        CAPTURE(first_pulse_on_id);

        // Step one full cycle (t=1.0) without any pulse_off
        r = runner.step_while(c1m::emptyt());
        REQUIRE_THAT(p.get_oscillator_phase(), p.phase_matcher(expected_pulse_on_phase));
        REQUIRE_THAT(r, m1m::equalst_on());
        auto second_pulse_on_id = *r.pulse_on_id();
        CAPTURE(second_pulse_on_id);

        // Step another full cycle (t=2.0): pulse off should match the first pulse
        r = runner.step_while(c1m::emptyt());

        // PROBLEMATIC EDGE CASE:
        //  We cannot be entirely sure here that the new pulse_on and old pulse_off will appear in the same time step.
        //  Since the legato value is close to 2.0 but not exactly 2.0, the new pulse_on might be produced in this
        //  cycle depending on rounding errors of the Oscillator's phase as well as the step size of the runner.

        if (r.last().voices()[0].size() == 1) {
            REQUIRE_THAT(r, m1m::equalst_off(first_pulse_on_id));
            r = runner.step();
            REQUIRE_THAT(r, m1m::equalst_on());
        } else {
            REQUIRE_THAT(r, m1m::sizet(2));
            REQUIRE_THAT(r, m1m::containst_off(first_pulse_on_id));
            REQUIRE_THAT(r, m1m::containst_on());
        }
        REQUIRE_THAT(r.time(), p.time_matcher(2.0));

        // Step another full cycle (t=3.0):
        r = runner.step_while(c1m::emptyt());
        
        if (r.last().voices()[0].size() == 1) {
            REQUIRE_THAT(r, m1m::equalst_off(second_pulse_on_id));
            r = runner.step();
            REQUIRE_THAT(r, m1m::equalst_on());
        } else {
            REQUIRE_THAT(r, m1m::sizet(2));
            REQUIRE_THAT(r, m1m::containst_off(second_pulse_on_id));
            REQUIRE_THAT(r, m1m::containst_on());
        }
        REQUIRE_THAT(r.time(), p.time_matcher(3.0));
    }
}

