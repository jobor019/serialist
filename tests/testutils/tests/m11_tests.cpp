#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
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

    SECTION("emptyf") {
         auto d_empty = RunResult<Facet>::dummy(Voices<Facet>::empty_like());
         REQUIRE_THAT(d_empty, m11::emptyf());

         auto d_non_empty = RunResult<Facet>::dummy(Voices<Facet>::singular(Facet{0.0}));
         REQUIRE_THAT(d_non_empty, !m11::emptyf());

     }

    SECTION("approx_eq, approx_eqf") {
         auto f = GENERATE(-100.0, -3.0, -1.0, -0.3, 0.3, 1.0, 3.0, 100.0);
         auto epsilon = GENERATE(1.0, 0.1, 0.01);

         auto d1 = RunResult<Facet>::dummy(Facet(f));
         auto d2 = RunResult<Facet>::dummy(Facet(f + 0.99 * epsilon));
         auto d3 = RunResult<Facet>::dummy(Facet(f - 0.99 * epsilon));
         REQUIRE_THAT(d1, m11::approx_eqf(f, epsilon));
         REQUIRE_THAT(d2, m11::approx_eqf(f, epsilon));
         REQUIRE_THAT(d3, m11::approx_eqf(f, epsilon));

         auto d4 = RunResult<Facet>::dummy(Facet(f + 1.01 * epsilon));
         auto d5 = RunResult<Facet>::dummy(Facet(f - 1.01 * epsilon));
         REQUIRE_THAT(d4, !m11::approx_eqf(f, epsilon));
         REQUIRE_THAT(d5, !m11::approx_eqf(f, epsilon));
     }

    SECTION("circular_eq, circular_eqf") {
         REQUIRE_THAT(RunResult<Facet>::dummy(Facet{1.0}), m11::circular_eqf(0.0, EPSILON));
         REQUIRE_THAT(RunResult<Facet>::dummy(Facet{0.99}), m11::circular_eqf(0.0, 0.02));
         REQUIRE_THAT(RunResult<Facet>::dummy(Facet{0.7}), m11::circular_eqf(0.1, 0.5));
         REQUIRE_THAT(RunResult<Facet>::dummy(Facet{0.5}), m11::circular_eqf(0.0, EPSILON, 0.5));

         REQUIRE_THAT(RunResult<Facet>::dummy(Facet{0.99}), !m11::circular_eqf(0.0, EPSILON));
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