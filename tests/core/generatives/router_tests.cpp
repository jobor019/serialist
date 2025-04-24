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
    w.mode.set_value(router::Mode::route);
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
    RouterFacetWrapper w(3, 2);
    w.mode.set_value(router::Mode::route);
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

        REQUIRE(multi_r.size() == 2);
        REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[0]), m11::eqf(111.0));
        REQUIRE_THAT(RunResult<Facet>::dummy(multi_r[1]), m11::eqf(222.0));
    }
}


TEST_CASE("Router: through (single)", "[router]") {
    RouterFacetWrapper w(1, 1);
    w.mode.set_value(router::Mode::through);

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
    w.mode.set_value(router::Mode::through);

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
    w.mode.set_value(router::Mode::merge);
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
    w.mode.set_value(router::Mode::split);
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