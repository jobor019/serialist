#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "matchers/matchers_common.h"
#include "matchers/m11.h"

using namespace serialist::test;

TEST_CASE("testutils::m11 value comparison", "[testutils][m11]") {
     SECTION("eqf") {
         auto f = GENERATE(-1e8, -100.0, -3.0, -1.0, -0.3, 0.3, 1.0, 3.0, 100.0, 1e8);
         auto d1 = RunResult<Facet>::dummy(Facet(f));

         REQUIRE_THAT(d1, m11::eqf(f));
         REQUIRE_THAT(d1, m11::eqf(f + Facet::ENUM_EPSILON / 2.0));
         REQUIRE_THAT(d1, m11::eqf(f - Facet::ENUM_EPSILON / 2.0));
         REQUIRE_THAT(d1, !m11::eqf(f + Facet::ENUM_EPSILON * 2));
         REQUIRE_THAT(d1, !m11::eqf(f - Facet::ENUM_EPSILON * 2));
         REQUIRE_THAT(d1, !m11::eqf(-f));
     }

    SECTION("ge/gef") {
        REQUIRE_THAT(RunResult<double>::dummy(0.1), m11::ge(0.0));
        REQUIRE_THAT(RunResult<double>::dummy(0.1), !m11::ge(0.2));

        REQUIRE_THAT(RunResult<Facet>::dummy(Facet(1.0)), m11::gef(0.9));
        REQUIRE_THAT(RunResult<Facet>::dummy(Facet(1.0)), m11::gef(1.0));
        REQUIRE_THAT(RunResult<Facet>::dummy(Facet(1.0)), !m11::gef(2.0));

        auto d1 = RunResult<Facet>::dummy(Vec<double>::linspace(0., 1.0, 10).as_type<Facet>());
        REQUIRE_THAT(d1, m11::gef(0.0, MatchType::all));
        REQUIRE_THAT(d1, !m11::gef(0.5, MatchType::all));
        REQUIRE_THAT(d1, m11::gef(0.5, MatchType::any));
        REQUIRE_THAT(d1, !m11::gef(1.1, MatchType::any));
    }

    SECTION("in_rangef") {
        auto f = GENERATE(-1e8, -100.0, -3.0, -1.0, -0.3, 0.3, 1.0, 3.0, 100.0, 1e8);
        auto d1 = RunResult<Facet>::dummy(Facet(f));

        REQUIRE_THAT(d1, m11::in_rangef(f - EPSILON, f + EPSILON));
        REQUIRE_THAT(d1, m11::in_rangef(f, f + EPSILON));
        REQUIRE_THAT(d1, m11::in_rangef(f - EPSILON, f, MatchType::last, false, true));

        REQUIRE_THAT(d1, !m11::in_rangef(f + EPSILON, f + 2 * EPSILON));
        REQUIRE_THAT(d1, !m11::in_rangef(f - 2 * EPSILON, f - EPSILON));
    }
}


TEST_CASE("testutils::m11 change comparison", "[testutils][m11]") {
    SECTION("increasing and decreasing") {
        auto strictly_increasing = RunResult<Facet>::dummy(Vec<double>::linspace(0., 1.0, 10).as_type<Facet>());
        REQUIRE_THAT(strictly_increasing, m11::strictly_increasingf());
        REQUIRE_THAT(strictly_increasing, m11::increasingf());
        REQUIRE_THAT(strictly_increasing, !m11::strictly_decreasingf());
        REQUIRE_THAT(strictly_increasing, !m11::decreasingf());

        auto increasing = RunResult<Facet>::dummy(Vec{0.0, 0.1, 0.2, 0.2, 0.3}.as_type<Facet>());
        REQUIRE_THAT(increasing, !m11::strictly_increasingf());
        REQUIRE_THAT(increasing, m11::increasingf());
        REQUIRE_THAT(increasing, !m11::strictly_decreasingf());
        REQUIRE_THAT(increasing, !m11::decreasingf());

        auto strictly_decreasing = RunResult<Facet>::dummy(Vec<double>::linspace(1.0, 0.0, 10).as_type<Facet>());
        REQUIRE_THAT(strictly_decreasing, !m11::strictly_increasingf());
        REQUIRE_THAT(strictly_decreasing, !m11::increasingf());
        REQUIRE_THAT(strictly_decreasing, m11::strictly_decreasingf());
        REQUIRE_THAT(strictly_decreasing, m11::decreasingf());

        auto decreasing = RunResult<Facet>::dummy(Vec{0.3, 0.2, 0.2, 0.1, 0.0}.as_type<Facet>());
        REQUIRE_THAT(decreasing, !m11::strictly_increasingf());
        REQUIRE_THAT(decreasing, !m11::increasingf());
        REQUIRE_THAT(decreasing, !m11::strictly_decreasingf());
        REQUIRE_THAT(decreasing, m11::decreasingf());

        auto neither = RunResult<Facet>::dummy(Vec{0.3, 0.2, 0.3, 0.1, 0.4}.as_type<Facet>());
        REQUIRE_THAT(neither, !m11::strictly_increasingf());
        REQUIRE_THAT(neither, !m11::increasingf());
        REQUIRE_THAT(neither, !m11::strictly_decreasingf());
        REQUIRE_THAT(neither, !m11::decreasingf());

        // Considered false (which might be slightly misleading) since there's not enough output to compare
        auto singular = RunResult<Facet>::dummy(Facet(0.3));
        REQUIRE_THAT(singular, !m11::strictly_increasingf());
        REQUIRE_THAT(singular, !m11::increasingf());
        REQUIRE_THAT(singular, !m11::strictly_decreasingf());
        REQUIRE_THAT(singular, !m11::decreasingf());
    }
}