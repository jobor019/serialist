#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/generatives/triggered_pulsator.h"

TEST_CASE("TriggeredPulsator ctor") {
    TriggeredPulsator p;
}

static TriggeredPulsator init_pulsator(double duration, double time) {
    TriggeredPulsator p;
    p.set_duration(duration);
    p.start(time, std::nullopt);
    return p;
}

const inline double epsilon = 1e-8;

TEST_CASE("TriggeredPulsator: non-overlapping pulses") {
    double duration = 2.0;
    double time = 0.0;

    TriggeredPulsator p = init_pulsator(duration, time);

    for (std::size_t i = TriggerIds::get_instance().peek_next_id(); i < 10; ++i) {
        auto triggers = p.handle_external_triggers(time, Voice<Trigger>::singular(Trigger::pulse_on()));
        REQUIRE(triggers.size() == 1);
        REQUIRE(Trigger::contains_pulse_on(triggers, i));

        REQUIRE(p.poll(time + epsilon).empty());

        time += duration;
        REQUIRE(p.poll(time - epsilon).empty());

        triggers = p.poll(time + epsilon);
        REQUIRE(triggers.size() == 1);
        REQUIRE(Trigger::contains_pulse_off(triggers, i));
    }
}


