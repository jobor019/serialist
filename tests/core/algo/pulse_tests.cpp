
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <iostream>
#include "core/temporal/pulse.h"

using namespace serialist;


TEST_CASE("PulseBroadcastHandler: basic operation", "[pulse][pulse_broadcast_handler]") {
    PulseBroadcastHandler h;

    auto on0 = Trigger::pulse_on();
    auto on1 = Trigger::pulse_on();
    auto off0 = Trigger::pulse_off(on0.get_id());

    auto v1a = Voices<Trigger>::singular(on0);
    auto v2a = Voices<Trigger>::transposed({on0, on1});
    auto v4a = Voices<Trigger>{{on0}, {}, {on1}, {off0}};

    auto v1b = v1a.cloned();
    auto v2b = v2a.cloned();
    auto v4b = v4a.cloned();

    auto v1ref = v1a.cloned();
    auto v2ref = v2a.cloned();
    auto v4ref = v4a.cloned();


    // SECTION("Unit broadcasting") {
    //     auto mask = h.broadcast(v1a, 1);
    //     REQUIRE(v1a == v1ref);
    //
    //     // Equal changes in size and number of voices shouldn't trigger broadcasting
    //     mask = h.broadcast(v2a, 2);
    //     REQUIRE(mask.empty());
    //     REQUIRE(v2a == v2ref);
    //
    //     mask = h.broadcast(v4a, 4);
    //     REQUIRE(mask.empty());
    //     REQUIRE(v4a == v4ref);
    // }

    // TODO: Rest of the tests
}
