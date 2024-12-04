#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "matchers/matchers_common.h"
#include "matchers/v11.h"

using namespace serialist::test;

TEST_CASE("testutils::v11", "[testutils][v11]") {
    auto f = GENERATE(-1e-4, -100.0, -3.0, -1.0, -0.3, 0.3, 1.0, 3.0, 100.0, 1e-8);

    auto d1 = RunResult<Facet>::dummy(Facet(f));

    SECTION("eqf") {
        REQUIRE_THAT(d1, v11::eqf(f));
        REQUIRE_THAT(d1, v11::eqf(f + Facet::ENUM_EPSILON / 2.0));
        REQUIRE_THAT(d1, v11::eqf(f - Facet::ENUM_EPSILON / 2.0));
        REQUIRE_THAT(d1, !v11::eqf(f + Facet::ENUM_EPSILON * 2));
        REQUIRE_THAT(d1, !v11::eqf(f - Facet::ENUM_EPSILON * 2));
    }
}
