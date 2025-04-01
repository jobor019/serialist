#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/policies/policies.h"
#include "core/types/index.h"
#include "matchers/matchers_common.h"

using namespace serialist;
using namespace serialist::test;


TEST_CASE("Index: index_op", "[index]") {
    SECTION("Phase 0 and 1") {
        std::size_t size = GENERATE(1, 10, 100, 1000, 1e4, 1e5, 1e6, 1e7);
        CAPTURE(size);

        // Phase 0 always maps to 0
        REQUIRE(Index::index_op(Phase::zero().get(), size) == 0);
        // Phase 1 always maps to size - 1
        REQUIRE(Index::index_op(Phase::one().get(), size) == size - 1);
    }

    SECTION("Position is floored, not rounded") {
        std::size_t size = 10;
        REQUIRE(Index::index_op(0.0, size) == 0);
        REQUIRE(Index::index_op(0.09, size) == 0);
        REQUIRE(Index::index_op(0.1 + EPSILON, size) == 1);
        REQUIRE(Index::index_op(0.49, size) == 4);
        REQUIRE(Index::index_op(0.5 + EPSILON, size) == 5);
        REQUIRE(Index::index_op(0.51 + EPSILON, size) == 5);
        REQUIRE(Index::index_op(0.99, size) == 9);
    }

    SECTION("Intermediate steps do not yield rounding errors") {
        std::size_t size = GENERATE(10, 100, 1000, 1e4, 1e5, 1e6);
        CAPTURE(size);

        SECTION("Phase") {
            for (std::size_t i = 0; i < size - 2; ++i) {
                auto phase = Phase(static_cast<double>(i) / size);

                REQUIRE(Index::index_op(phase.get(), size) == i);
            }
        }

        SECTION("Phase-like values") {
            for (std::size_t i = 0; i < size - 2; ++i) {
                auto phase_like = static_cast<double>(i) / size;
                CAPTURE(phase_like);
                REQUIRE(Index::index_op(phase_like, size) == i);
            }
        }
    }

    SECTION("Cursor positions greater than 1 yields multiples of size") {
        std::size_t size = GENERATE(10, 100, 1000, 10000);
        std::size_t multiplier = GENERATE(2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 127);
        CAPTURE(size, multiplier);

        for (std::size_t i = 0; i < multiplier * size; ++i) {
            auto cursor = static_cast<double>(i) / size;
            CAPTURE(i, cursor);
            REQUIRE(Index::index_op(cursor, size) == i);
        }
    }
}


TEST_CASE("Index: Conversion from Facet", "[index]") {
    auto max = 1e7;
    for (std::size_t i = 0; i < max; ++i) {
        auto closest_double = static_cast<double>(i);
        REQUIRE(Index::from_index_facet(closest_double).get_raw() == i);
    }
}
