#include "core/policies/policies.h"
#include "node_runner.h"
#include "generatives/router.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>


#include "generators.h"
#include "matchers/m1m.h"
#include "matchers/m11.h"
#include "matchers/m1s.h"
#include "matchers/mms.h"

using namespace serialist;
using namespace serialist::test;

/**
 *  Since NodeRunner currently doesn't support MultiNodes, all test here are manual.
 *  Fortunately, Router's behaviour is independent of time, so there's no need to test anything beyond the initial time.
 */

void set_map(Sequence<Facet, double>& map_node, std::initializer_list<double> values) {
    map_node.set_values(Voices<double>::transposed(Voice<double>{values}));
}


TEST_CASE("Router: route (single)", "[router]") {
    RouterFacetWrapper w(1, 1);
    w.mode.set_value(RouterMode::route);
    w.uses_index.set_value(true);

    w.set_input(0, Voices<double>::transposed(Voice<double>{111, 222, 333}));

    auto& router = w.router_node;
    auto& map = w.routing_map;

    SECTION("Unit map") {
        set_map(map, {0.0, 1.0, 2.0});
        router.update_time(TimePoint{});
        auto multi_r = router.process();

        REQUIRE(multi_r.size() == 1);
        auto r = RunResult<Facet>::dummy(multi_r[0]);
        REQUIRE_THAT(r, m1s::eqf(Vec{111.0, 222.0, 333.0}));
    }

    SECTION("Subset") {
        set_map(map, {0.0, 1.0});
        router.update_time(TimePoint{});
        auto multi_r = router.process();

        REQUIRE(multi_r.size() == 1);
        auto r = RunResult<Facet>::dummy(multi_r[0]);
        REQUIRE_THAT(r, m1s::eqf(Vec{111.0, 222.0}));
    }
}


TEST_CASE("Router: route (multi)", "[router]") {
    RouterFacetWrapper w(3, 3);
    w.mode.set_value(RouterMode::route);
    w.uses_index.set_value(true);

    w.set_input(0, Voices<double>::singular(111));
    w.set_input(1, Voices<double>::singular(222));
    w.set_input(2, Voices<double>::singular(333));

    auto& router = w.router_node;
    auto& map = w.routing_map;

    SECTION("Unit map") {
        set_map(map, {0.0, 1.0, 2.0});
        router.update_time(TimePoint{});
        auto multi_r = router.process();

        REQUIRE(multi_r.size() == 3);
        REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[0]), m11::eqf(111.0));
        REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[1]), m11::eqf(222.0));
        REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[2]), m11::eqf(333.0));
    }

    SECTION("Subset") {
        set_map(map, {0.0, 1.0});
        router.update_time(TimePoint{});
        auto multi_r = router.process();

        REQUIRE(multi_r.size() == 3);
        REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[0]), m11::eqf(111.0));
        REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[1]), m11::eqf(222.0));
        REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[2]), m11::emptyf());
    }
}


TEST_CASE("Router: through (single)", "[router]") {
    RouterFacetWrapper w(1, 1);
    w.mode.set_value(RouterMode::through);

    w.set_input(0, Voices<double>::transposed(Voice<double>{111, 222, 333}));

    auto& router = w.router_node;
    auto& map = w.routing_map;

    SECTION("Unit map") {
        set_map(map, {1, 1, 1}); // boolean mask
        router.update_time(TimePoint{});
        auto multi_r = router.process();

        REQUIRE(multi_r.size() == 1);
        auto r = RunResult<Facet>::dummy(multi_r[0]);
        REQUIRE_THAT(r, m1s::eqf(Vec{111.0, 222.0, 333.0}));
    }

    SECTION("Subset") {
        set_map(map, {1, 1, 0});
        router.update_time(TimePoint{});
        auto multi_r = router.process();

        REQUIRE(multi_r.size() == 1);
        auto r = multi_r[0];
        REQUIRE(r.size() == 3);
        REQUIRE(r[0].size() == 1);
        REQUIRE_THAT(r[0][0], Catch::Matchers::WithinAbs(111.0, 1e-8));
        REQUIRE(r[1].size() == 1);
        REQUIRE_THAT(r[1][0], Catch::Matchers::WithinAbs(222.0, 1e-8));

        REQUIRE(r[2].empty());
    }
}


TEST_CASE("Router: through (multi)", "[router]") {
    RouterFacetWrapper w(3, 3);
    w.mode.set_value(RouterMode::through);

    w.set_input(0, Voices<double>::singular(111));
    w.set_input(1, Voices<double>::singular(222));
    w.set_input(2, Voices<double>::singular(333));

    auto& router = w.router_node;
    auto& map = w.routing_map;

    SECTION("Unit map") {
        set_map(map, {1, 1, 1});
        router.update_time(TimePoint{});
        auto multi_r = router.process();

        REQUIRE(multi_r.size() == 3);
        REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[0]), m11::eqf(111.0));
        REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[1]), m11::eqf(222.0));
        REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[2]), m11::eqf(333.0));
    }

    SECTION("Subset") {
        set_map(map, {1, 1, 0});
        router.update_time(TimePoint{});
        auto multi_r = router.process();

        REQUIRE(multi_r.size() == 3);
        REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[0]), m11::eqf(111.0));
        REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[1]), m11::eqf(222.0));
        REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[2]), m11::emptyf());
    }
}


TEST_CASE("Router: merge", "[router]") {
    RouterFacetWrapper w(3, 1);
    w.mode.set_value(RouterMode::merge);
    w.uses_index.set_value(true);

    w.set_input(0, Voices<double>::transposed(Voice<double>{111, 222, 333}));
    w.set_input(1, Voices<double>::transposed(Voice<double>{444, 555}));
    w.set_input(2, Voices<double>::singular(666));

    auto& router = w.router_node;
    auto& map = w.routing_map;

    set_map(map, {2, 1, 0}); // 2 from first voice, 1 from 2nd, 0 from 3rd

    router.update_time(TimePoint{});
    auto multi_r = router.process();

    REQUIRE(multi_r.size() == 1);
    REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[0]), m1s::eqf(Vec{111.0, 222.0, 444.0}));
}


TEST_CASE("Router: split", "[router]") {
    RouterFacetWrapper w(1, 3);
    w.mode.set_value(RouterMode::split);
    w.uses_index.set_value(true);

    w.set_input(0, Voices<double>::transposed(Voice<double>{111, 222, 333, 444, 555}));

    auto& router = w.router_node;
    auto& map = w.routing_map;

    set_map(map, {2, 2, 0}); // 2 to first outlet, 2 to 2nd, 0 to 3rd

    router.update_time(TimePoint{});
    auto multi_r = router.process();

    REQUIRE(multi_r.size() == 3);
    REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[0]), m1s::eqf(Vec{111.0, 222.0}));
    REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[1]), m1s::eqf(Vec{333.0, 444.0}));
    REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[2]), m11::emptyf());
}

TEST_CASE("Router: mix", "[router]") {
    RouterFacetWrapper w(3, 1);
    w.mode.set_value(RouterMode::mix);
    w.uses_index.set_value(true);

    w.set_input(0, Voices<double>::transposed(Voice<double>{111, 222, 333}));
    w.set_input(1, Voices<double>::transposed(Voice<double>{444, 555}));
    w.set_input(2, Voices<double>::singular(666));

    auto& router = w.router_node;
    auto& map = w.routing_map;

    auto mix_spec = Voices<double> {{
            Voice<double>{0, 0}, // first element in first inlet
            Voice<double>{0, 1}, // second element in first inlet
            Voice<double>{1, 0}, // first element in second inlet
            Voice<double>{0, 0}, // first/first again
            Voice<double>{0, 2}  // third element in first inlet
    }};

    map.set_values(mix_spec);


    router.update_time(TimePoint{});
    auto multi_r = router.process();

    REQUIRE(multi_r.size() == 1);
    REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[0]), m1s::eqf(Vec{111.0, 222.0, 444.0, 111.0, 333.0}));
}


TEST_CASE("Router: distribute", "[router]") {
    RouterFacetWrapper w(1, 3);
    w.mode.set_value(RouterMode::distribute);
    w.uses_index.set_value(true);

    w.set_input(0, Voices<double>::transposed(Voice<double>{111, 222, 333, 444, 555}));

    auto& router = w.router_node;
    auto& map = w.routing_map;

    auto distribute_spec = Voices<double> { {
        Voice<double>{0, 1, 3}, // first outlet
        Voice<double>{0, 2, 2}, // second outlet
        Voice<double>{}         // third outlet
    }};

    map.set_values(distribute_spec);

    router.update_time(TimePoint{});
    auto multi_r = router.process();

    REQUIRE(multi_r.size() == 3);
    REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[0]), m1s::eqf(Vec{111.0, 222.0, 444.0}));
    REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[1]), m1s::eqf(Vec{111.0, 333.0, 333.0}));
    REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[2]), m11::emptyf());
}


// ==============================================================================================
// PULSE TESTS
// ==============================================================================================

TEST_CASE("Router: route pulse (single)", "[router]") {
    RouterPulseWrapper w(1, 1);

    w.mode.set_value(RouterMode::route);
    w.uses_index.set_value(true);

    w.set_input(0, Voices<Trigger>{{Trigger::pulse_on()}, {}, {Trigger::pulse_on()}});;

    auto& router = w.router_node;
    auto& map = w.routing_map;

    SECTION("Unit map") {
        set_map(map, {0.0, 1.0, 2.0});
        router.update_time(TimePoint{});
        auto multi_r = router.process();

        REQUIRE(multi_r.size() == 1);
        auto r = RunResult<Trigger>::dummy(multi_r[0]);

        REQUIRE_THAT(r, mms::sizet(3));
        REQUIRE_THAT(r, mms::equalst_on(0));
        REQUIRE_THAT(r, mms::emptyt(1));
        REQUIRE_THAT(r, mms::equalst_on(2));

        w.set_input(0, Voices<Trigger>{{}, {Trigger::pulse_on()}, {}});;

        router.update_time(TimePoint{});
        multi_r = router.process();

        REQUIRE(multi_r.size() == 1);
        r = RunResult<Trigger>::dummy(multi_r[0]);

        REQUIRE_THAT(r, mms::sizet(3));
        REQUIRE_THAT(r, mms::emptyt(0));
        REQUIRE_THAT(r, mms::equalst_on(1));
        REQUIRE_THAT(r, mms::emptyt(2));
    }


    SECTION("Subset") {
        set_map(map, {1.0, 0.0});
        router.update_time(TimePoint{});
        auto multi_r = router.process();

        REQUIRE(multi_r.size() == 1);
        auto r = RunResult<Trigger>::dummy(multi_r[0]);

        REQUIRE_THAT(r, mms::sizet(2));
        REQUIRE_THAT(r, mms::emptyt(0));
        REQUIRE_THAT(r, mms::equalst_on(1));

        w.set_input(0, Voices<Trigger>{{}, {Trigger::pulse_on()}, {}});

        router.update_time(TimePoint{});
        multi_r = router.process();

        REQUIRE(multi_r.size() == 1);
        r = RunResult<Trigger>::dummy(multi_r[0]);

        REQUIRE_THAT(r, mms::sizet(2));
        REQUIRE_THAT(r, mms::equalst_on(0));
        REQUIRE_THAT(r, mms::emptyt(1));
    }

    SECTION("Mapping larger than input") {
        set_map(map, {0.0, 1.0, 2.0, 3.0});
        router.update_time(TimePoint{});
        auto multi_r = router.process();

        REQUIRE(multi_r.size() == 1);
        auto r = RunResult<Trigger>::dummy(multi_r[0]);

        REQUIRE_THAT(r, mms::sizet(3)); // clipped to input size
        REQUIRE_THAT(r, mms::equalst_on(0));
        REQUIRE_THAT(r, mms::emptyt(1));
        REQUIRE_THAT(r, mms::equalst_on(2));
    }

    SECTION("Mapping with closed voices") {
        map.set_values({{0.0}, {}, {2.0}});
        w.set_input(0, Voices<Trigger>{{Trigger::pulse_on()}, {Trigger::pulse_off(std::nullopt)}, {}});
        router.update_time(TimePoint{});

        // Initial state: [- - -]
        auto multi_r = router.process();

        REQUIRE(multi_r.size() == 1);
        auto r = RunResult<Trigger>::dummy(multi_r[0]);

        REQUIRE_THAT(r, mms::sizet(3));
        REQUIRE_THAT(r, mms::equalst_on(0));
        REQUIRE_THAT(r, mms::emptyt(1)); // closed voice
        REQUIRE_THAT(r, mms::emptyt(2));
    }

    SECTION("Empty mapping yields no output") {
        set_map(map, {});
        router.update_time(TimePoint{});
        auto multi_r = router.process();

        REQUIRE(multi_r.size() == 1);
        auto r = RunResult<Trigger>::dummy(multi_r[0]);
        REQUIRE_THAT(r, m11::empty<Trigger>());
    }

    SECTION("Flush") {
        set_map(map, {0.0, 1.0, 2.0});
        router.update_time(TimePoint{});
        router.process(); // ignoring output, assuming same as in SECTION("Unit Map")

        w.enabled.set_value(false);
        router.update_time(TimePoint{});
        auto multi_r = router.process();

        REQUIRE(multi_r.size() == 1);
        auto r = RunResult<Trigger>::dummy(multi_r[0]);

        REQUIRE_THAT(r, mms::sizet(3));
        REQUIRE_THAT(r, mms::equalst_off(0));
        REQUIRE_THAT(r, mms::emptyt(1));
        REQUIRE_THAT(r, mms::equalst_off(2));
    }
}


TEST_CASE("Router: route pulse (single), changing routing", "[router]") {
    RouterPulseWrapper w(1, 1);

    w.mode.set_value(RouterMode::route);
    w.uses_index.set_value(true);

    auto& router = w.router_node;
    auto& map = w.routing_map;
    auto& flush = w.flush_mode;

    set_map(map, {0.0, 1.0});

    SECTION("Change to other index flushes immediately (FlushMode::always") {
        flush.set_value(FlushMode::always);

        auto pulse_on = Trigger::pulse_on();
        // Initial state: [ON -]
        w.set_input(0, Voices<Trigger>{{pulse_on}, {}});
        router.update_time(TimePoint{});
        router.process();

        // Change voice 0 routing with no new input
        set_map(map, {1.0, 1.0});
        w.set_input(0, Voices<Trigger>{{}, {}});

        router.update_time(TimePoint{});
        router.process();
        auto multi_r = router.process();
        REQUIRE(multi_r.size() == 1);
        auto r = RunResult<Trigger>::dummy(multi_r[0]);

        // Expected flush of voice 0 (new state after flush: [- -])
        REQUIRE_THAT(r, mms::sizet(2));
        REQUIRE_THAT(r, mms::equalst_off(0, pulse_on.get_id()));
        REQUIRE_THAT(r, mms::emptyt(1));
    }

    SECTION("Change to other index flushes on next pulse (FlushMode::any_pulse)") {
        flush.set_value(FlushMode::any_pulse);

        auto pulse_on = Trigger::pulse_on();
        // Initial state: [ON(a) -]
        w.set_input(0, Voices<Trigger>{{pulse_on}, {}});
        router.update_time(TimePoint{});
        router.process();

        // Change voice 0 routing with no new input
        set_map(map, {1.0, 1.0});
        w.set_input(0, Voices<Trigger>{{}, {}});

        router.update_time(TimePoint{});
        router.process();
        auto multi_r = router.process();
        REQUIRE(multi_r.size() == 1);
        auto r = RunResult<Trigger>::dummy(multi_r[0]);

        // No flush of voice 0, but flag its active ON as triggered (new state after flush: [ON(a, triggered) -])
        REQUIRE_THAT(r, mms::sizet(2));
        REQUIRE_THAT(r, mms::emptyt(0));
        REQUIRE_THAT(r, mms::emptyt(1));

        // Pulse input on voice 1 (which is routed to voice 0). note: not a pulse_off,
        // but we still expect a flush of the triggered ON(a)
        w.set_input(0, Voices<Trigger>{{}, {Trigger::pulse_on()}});
        router.update_time(TimePoint{});
        router.process();
        multi_r = router.process();
        REQUIRE(multi_r.size() == 1);
        r = RunResult<Trigger>::dummy(multi_r[0]);

        // Expected flush of voice 0 (new state after flush: [ON(b) ON(b)])
        REQUIRE_THAT(r, mms::sizet(2));
        REQUIRE_THAT(r, mms::equalst_off({0, 0}, pulse_on.get_id()));
        REQUIRE_THAT(r, mms::equalst_on({0, 1}));
        REQUIRE_THAT(r, mms::equalst_on(1));
    }

    SECTION("Change to other index with multiple active pulses flushes all pulses") {
        flush.set_value(FlushMode::always);

        auto pulse_on_a = Trigger::pulse_on();

        w.set_input(0, Voices<Trigger>{{pulse_on_a}, {}});
        router.update_time(TimePoint{});
        router.process(); // Initial state: [ON(a) -]

        auto pulse_on_b = Trigger::pulse_on();
        w.set_input(0, Voices<Trigger>{{pulse_on_b}, {}});
        router.update_time(TimePoint{});
        router.process(); // Updated state: [ON(a)ON(b) -]

        // Change voice 0 routing with no new input
        set_map(map, {1.0, 1.0});
        w.set_input(0, Voices<Trigger>{{}, {}});

        router.update_time(TimePoint{});
        router.process();
        auto multi_r = router.process();
        REQUIRE(multi_r.size() == 1);
        auto r = RunResult<Trigger>::dummy(multi_r[0]);

        // Expected flush of both pulses in voice 0 (new state after flush: [- -])
        REQUIRE_THAT(r, mms::sizet(2));
        REQUIRE_THAT(r, mms::sizet(0, 2));
        REQUIRE_THAT(r, mms::containst_off(0, pulse_on_a.get_id()));
        REQUIRE_THAT(r, mms::containst_off(0, pulse_on_b.get_id()));
        REQUIRE_THAT(r, mms::emptyt(1));
    }


    SECTION("Change to other index with no active pulses does not flush anything") {
        w.set_input(0, Voices<Trigger>{{}, {}});
        router.update_time(TimePoint{});
        router.process(); // Initial state: [- -]

        set_map(map, {1.0, 1.0});
        w.set_input(0, Voices<Trigger>{{}, {}});

        router.update_time(TimePoint{});
        router.process();
        auto multi_r = router.process();
        REQUIRE(multi_r.size() == 1);
        auto r = RunResult<Trigger>::dummy(multi_r[0]);

        REQUIRE_THAT(r, mms::sizet(2));
        REQUIRE_THAT(r, mms::emptyt(0));
        REQUIRE_THAT(r, mms::emptyt(1));
    }


    SECTION("Closing an outlet flushes all active pulses independently of FlushMode") {
        auto flush_mode = GENERATE(FlushMode::always, FlushMode::any_pulse);
        flush.set_value(flush_mode);

        auto pulse_on = Trigger::pulse_on();
        w.set_input(0, Voices<Trigger>{{pulse_on}, {}});
        router.update_time(TimePoint{});
        router.process(); // Initial state: [ON -]

        // Closing first outlet
        map.set_values({{}, {1.0}});
        w.set_input(0, Voices<Trigger>{{}, {}});

        router.update_time(TimePoint{});
        router.process();
        auto multi_r = router.process();
        REQUIRE(multi_r.size() == 1);
        auto r = RunResult<Trigger>::dummy(multi_r[0]);

        REQUIRE_THAT(r, mms::sizet(2));
        REQUIRE_THAT(r, mms::containst_off(0, pulse_on.get_id()));
        REQUIRE_THAT(r, mms::emptyt(1));
    }

    SECTION("Change to empty mapping flushes all active pulses") {
        auto flush_mode = GENERATE(FlushMode::always, FlushMode::any_pulse);
        flush.set_value(flush_mode);

        auto pulse_on_a = Trigger::pulse_on();
        auto pulse_on_b = Trigger::pulse_on();
        w.set_input(0, Voices<Trigger>{ {pulse_on_a}, {pulse_on_b} });
        router.update_time(TimePoint{});
        router.process(); // Initial state: [ON(a) ON(b)]


        map.set_values(Voices<double>::empty_like());
        w.set_input(0, Voices<Trigger>{{}, {}});

        router.update_time(TimePoint{});
        auto multi_r = router.process();
        REQUIRE(multi_r.size() == 1);
        auto r = RunResult<Trigger>::dummy(multi_r[0]);

        REQUIRE_THAT(r, mms::sizet(2));
        REQUIRE_THAT(r, mms::equalst_off(0, pulse_on_a.get_id()));
        REQUIRE_THAT(r, mms::equalst_off(1, pulse_on_b.get_id()));
    }


    SECTION("Empty input on a single-voice inlet does not flush") {
        auto pulse_on = Trigger::pulse_on();


        map.set_values(0.0);
        w.set_input(0, Voices<Trigger>::singular(pulse_on));

        router.update_time(TimePoint{});
        auto multi_r = router.process();
        REQUIRE(multi_r.size() == 1);
        auto r = RunResult<Trigger>::dummy(multi_r[0]);

        REQUIRE_THAT(r, mms::sizet(1));
        REQUIRE_THAT(r, mms::containst_on(0));

        // State: [ON]
        w.set_input(0, Voices<Trigger>::empty_like());
        router.update_time(TimePoint{});
        multi_r = router.process();
        REQUIRE(multi_r.size() == 1);
        r = RunResult<Trigger>::dummy(multi_r[0]);

        // No input: state still [ON]
        REQUIRE_THAT(r, mms::sizet(1));
        REQUIRE_THAT(r, mms::emptyt(0));

        // Close outlet, expect flush
        map.set_values(Voices<double>::empty_like());

        router.update_time(TimePoint{});
        multi_r = router.process();
        REQUIRE(multi_r.size() == 1);
        r = RunResult<Trigger>::dummy(multi_r[0]);

        REQUIRE_THAT(r, mms::sizet(1));
        REQUIRE_THAT(r, mms::equalst_off(0, pulse_on.get_id()));
    }
}


TEST_CASE("Router: route pulse (single), switching mode", "[router]") {
    RouterPulseWrapper w(1, 1);

    w.mode.set_value(RouterMode::route);
    w.uses_index.set_value(true);

    auto& router = w.router_node;
    auto& map = w.routing_map;
    auto& flush = w.flush_mode;

    set_map(map, {0.0, 1.0});

    SECTION("Switching mode correctly flushes all voices") {
        auto flush_mode = GENERATE(FlushMode::always, FlushMode::any_pulse);
        flush.set_value(flush_mode);

        auto pulse_on_voice0 = Trigger::pulse_on();
        auto pulse_on_voice1 = Trigger::pulse_on();
        CAPTURE(pulse_on_voice0.get_id(), pulse_on_voice1.get_id());

        w.set_input(0, Voices<Trigger>{{pulse_on_voice0}, {pulse_on_voice1}});
        router.update_time(TimePoint{});
        auto multi_r = router.process();
        REQUIRE(multi_r.size() == 1);
        auto r = RunResult<Trigger>::dummy(multi_r[0]);

        // Initial state: [ON(a) ON(b)]
        REQUIRE_THAT(r, mms::sizet(2));
        REQUIRE_THAT(r, mms::equalst_on(0, pulse_on_voice0.get_id()));
        REQUIRE_THAT(r, mms::equalst_on(1, pulse_on_voice1.get_id()));

        w.mode.set_value(RouterMode::through);

        map.set_values({{}, {}});

        router.update_time(TimePoint{});
        router.process();
        multi_r = router.process();
        REQUIRE(multi_r.size() == 1);
        r = RunResult<Trigger>::dummy(multi_r[0]);

        REQUIRE_THAT(r, mms::sizet(2));
        REQUIRE_THAT(r, mms::equalst_off(0, pulse_on_voice0.get_id()));
        REQUIRE_THAT(r, mms::equalst_off(1, pulse_on_voice1.get_id()));

    }
}


TEST_CASE("Router: route pulse (single), resizing input", "[router]") {
    RouterPulseWrapper w(1, 1);

    w.mode.set_value(RouterMode::route);
    w.uses_index.set_value(true);

    auto& router = w.router_node;
    auto& map = w.routing_map;

    // Unit map (fixed throughout entire test)
    set_map(map, {0.0, 1.0, 2.0});

    // Initial input: [ON ON ON] (size 3, same as map)
    w.set_input(0, Voices<Trigger>{{Trigger::pulse_on()}, {Trigger::pulse_on()}, {Trigger::pulse_on()}});
    router.update_time(TimePoint{});
    auto multi_r = router.process(); // Initial state: [ON ON ON]

    // Change size of input below size of map
    w.set_input(0, Voices<Trigger>{{}, {}});
    router.update_time(TimePoint{});
    multi_r = router.process();
    REQUIRE(multi_r.size() == 1);
    auto r = RunResult<Trigger>::dummy(multi_r[0]);

    // We expect third voice to be flushed here: output: [- - OFF], new state: [ON ON]
    REQUIRE_THAT(r, mms::sizet(3));
    REQUIRE_THAT(r, mms::emptyt(0));
    REQUIRE_THAT(r, mms::emptyt(1));
    REQUIRE_THAT(r, mms::equalst_off(2));

    // Change size back to size of map
    w.set_input(0, Voices<Trigger>{{}, {}, {Trigger::pulse_on()}});
    router.update_time(TimePoint{});
    multi_r = router.process();
    REQUIRE(multi_r.size() == 1);
    r = RunResult<Trigger>::dummy(multi_r[0]);

    // output: [- - ON], new state: [ON ON ON]
    REQUIRE_THAT(r, mms::sizet(3));
    REQUIRE_THAT(r, mms::emptyt(0));
    REQUIRE_THAT(r, mms::emptyt(1));
    REQUIRE_THAT(r, mms::equalst_on(2));

    // Change size above size of map (extra voice should be ignored)
    w.set_input(0, Voices<Trigger>{{}, {}, {}, {Trigger::pulse_on()}});
    router.update_time(TimePoint{});
    multi_r = router.process();
    REQUIRE(multi_r.size() == 1);
    r = RunResult<Trigger>::dummy(multi_r[0]);

    // output: [- - -], no change in state
    REQUIRE_THAT(r, m11::empty<Trigger>());

    // Change size back to size of map
    w.set_input(0, Voices<Trigger>{{}, {}, {}});
    router.update_time(TimePoint{});
    multi_r = router.process();
    REQUIRE(multi_r.size() == 1);
    r = RunResult<Trigger>::dummy(multi_r[0]);

    // output: [- - -], no change in state
    REQUIRE_THAT(r, m11::empty<Trigger>());
}