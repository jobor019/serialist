#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "matchers/matchers_common.h"
#include "matchers/v11.h"

using namespace serialist::test;

TEST_CASE("testutils::v11", "[testutils][v11]") {
    SECTION("eqf") {
        auto f = GENERATE(-1e8, -100.0, -3.0, -1.0, -0.3, 0.3, 1.0, 3.0, 100.0, 1e8);
        auto d1 = RunResult<Facet>::dummy(Facet(f));

        REQUIRE_THAT(d1, v11::eqf(f));
        REQUIRE_THAT(d1, v11::eqf(f + Facet::ENUM_EPSILON / 2.0));
        REQUIRE_THAT(d1, v11::eqf(f - Facet::ENUM_EPSILON / 2.0));
        REQUIRE_THAT(d1, !v11::eqf(f + Facet::ENUM_EPSILON * 2));
        REQUIRE_THAT(d1, !v11::eqf(f - Facet::ENUM_EPSILON * 2));
        REQUIRE_THAT(d1, !v11::eqf(-f));
    }

    SECTION("in_rangef") {
        auto f = GENERATE(-1e8, -100.0, -3.0, -1.0, -0.3, 0.3, 1.0, 3.0, 100.0, 1e8);
        auto d1 = RunResult<Facet>::dummy(Facet(f));

        REQUIRE_THAT(d1, v11::in_rangef(f - EPSILON, f + EPSILON));
        REQUIRE_THAT(d1, v11::in_rangef(f, f + EPSILON));
        REQUIRE_THAT(d1, v11::in_rangef(f - EPSILON, f, true));

        REQUIRE_THAT(d1, !v11::in_rangef(f + EPSILON, f + 2 * EPSILON));
        REQUIRE_THAT(d1, !v11::in_rangef(f - 2 * EPSILON, f - EPSILON));
    }
}


TEST_CASE("testutils::v11h", "[testutils][v11h]") {
    auto d1 = RunResult<Facet>::dummy(Facet(0.0)
                                      , Vec<double>::linspace(0., 1.0, 10).as_type<Facet>());

    REQUIRE_THAT(d1, v11h::strictly_increasing<Facet>());
    REQUIRE_THAT(d1, !v11h::strictly_decreasing<Facet>());
    REQUIRE_THAT(d1, v11h::allf(v11::gef(0.0)));
    REQUIRE_THAT(d1, !v11h::allf(v11::gef(0.5)));
    REQUIRE_THAT(d1, v11h::allf(v11::lef(1.0)));
    REQUIRE_THAT(d1, !v11h::allf(v11::lef(0.5)));
}
