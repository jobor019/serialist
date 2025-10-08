
#include "core/policies/policies.h"
#include "node_runner.h"
#include "generatives/pulse_filter.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "generators.h"
#include "matchers/m1m.h"

using namespace serialist;
using namespace serialist::test;

auto PAUSE = Voices<PulseFilter::State>::singular(PulseFilter::State::pause);
auto SUSTAIN = Voices<PulseFilter::State>::singular(PulseFilter::State::sustain);
auto OPEN = Voices<PulseFilter::State>::singular(PulseFilter::State::open);

auto NO_TRIGGER = Voices<Trigger>::empty_like();

std::pair<Voices<Trigger>, std::size_t> single_pulse_on() {
    auto trigger = Trigger::pulse_on();
    return {Voices<Trigger>::singular(trigger), trigger.get_id()};
}

Voices<Trigger> single_pulse_off(std::size_t id) {
    return Voices<Trigger>::singular(Trigger::pulse_off(id));
}

TEST_CASE("PulseFilter: Pause-Open", "[pulse_filter]") {
    PulseFilterWrapper<> w;
    auto& trigger = w.trigger;
    auto& filter_state = w.filter_state;

    auto runner = NodeRunner{&w.pulse_filter_node};

    filter_state.set_values(OPEN);

    SECTION("immediate") {
        w.immediate.set_value(true);

        // Initially open: passthrough
        auto [pulse_on1, id1] = single_pulse_on();
        trigger.set_values(pulse_on1);
        REQUIRE_THAT(runner.step(), m1m::equalst_on(id1));
        trigger.set_values(NO_TRIGGER); // need to reset inbound

        // Queue another trigger
        auto [pulse_on2, id2] = single_pulse_on();
        trigger.set_values(pulse_on2);
        REQUIRE_THAT(runner.step(), m1m::equalst_on(id2));

        // Intermediate step: no output
        trigger.set_values(NO_TRIGGER);
        REQUIRE_THAT(runner.step(), m1m::emptyt());

        // release pulse2
        trigger.set_values(single_pulse_off(id2));
        REQUIRE_THAT(runner.step(), m1m::equalst_off(id2));
        trigger.set_values(NO_TRIGGER);

        // Immediately output any remaining pulse_offs (id1) on first closed step
        filter_state.set_values(PAUSE);
        REQUIRE_THAT(runner.step(), m1m::equalst_off(id1));

        // Ignore any further pulse_ons & pulse_offs until we're back open
        auto [pulse_on3, id3] = single_pulse_on();
        trigger.set_values(pulse_on3);
        REQUIRE_THAT(runner.step(), m1m::emptyt());
        trigger.set_values(single_pulse_off(id3));
        REQUIRE_THAT(runner.step(), m1m::emptyt());
        trigger.set_values(NO_TRIGGER);

        // On open: no lingering pulses
        filter_state.set_values(OPEN);
        REQUIRE_THAT(runner.step(), m1m::emptyt());
    }
}


TEST_CASE("PulseFilter: Sustain mode", "[pulse_filter]") {
    PulseFilterWrapper<> w;
    auto& trigger = w.trigger;
    auto& filter_state = w.filter_state;

    auto runner = NodeRunner{&w.pulse_filter_node};

    filter_state.set_values(OPEN);

    SECTION("immediate") {
        w.immediate.set_value(true);

        // Initially open: passthrough
        auto [pulse_on1, id1] = single_pulse_on();
        trigger.set_values(pulse_on1);
        REQUIRE_THAT(runner.step(), m1m::equalst_on(id1));
        trigger.set_values(NO_TRIGGER); // need to reset inbound

        // Queue another trigger
        auto [pulse_on2, id2] = single_pulse_on();
        trigger.set_values(pulse_on2);
        REQUIRE_THAT(runner.step(), m1m::equalst_on(id2));

        // Intermediate step: no output
        trigger.set_values(NO_TRIGGER);
        REQUIRE_THAT(runner.step(), m1m::emptyt());

        // release pulse2
        trigger.set_values(single_pulse_off(id2));
        REQUIRE_THAT(runner.step(), m1m::equalst_off(id2));
        trigger.set_values(NO_TRIGGER);

        // Close filter: no particular action
        filter_state.set_values(SUSTAIN);
        REQUIRE_THAT(runner.step(), m1m::emptyt());

        // Queue pulse_off for id1: sustain until opened again
        trigger.set_values(single_pulse_off(id1));
        REQUIRE_THAT(runner.step(), m1m::emptyt());
        trigger.set_values(NO_TRIGGER);

        // Ignore any further pulse_ons & pulse_offs until we're back open
        auto [pulse_on3, id3] = single_pulse_on();
        trigger.set_values(pulse_on3);
        REQUIRE_THAT(runner.step(), m1m::emptyt());
        trigger.set_values(single_pulse_off(id3));
        REQUIRE_THAT(runner.step(), m1m::emptyt());
        trigger.set_values(NO_TRIGGER);

        // On open: output lingering pulse_offs
        filter_state.set_values(OPEN);
        REQUIRE_THAT(runner.step(), m1m::equalst_off(id1));
    }
}