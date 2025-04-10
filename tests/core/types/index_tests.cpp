#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/policies/policies.h"
#include "core/types/index.h"
#include "matchers/matchers_common.h"

using namespace serialist;
using namespace serialist::test;


TEST_CASE("Index: index_op (positive indices)", "[index]") {
    SECTION("Phase 0 and 1") {
        // std::size_t size = GENERATE(1, 10, 100, 1000, 1e4, 1e5, 1e6, 1e7);
        std::size_t size = GENERATE(10);
        CAPTURE(size);

        // Phase 0 always maps to 0
        REQUIRE(Index::index_op(Phase::zero().get(), size) == 0);
        // Phase 1 always maps to size - 1
        REQUIRE(Index::index_op(Phase::one().get(), size) == size - 1);
    }

    SECTION("Container size 0 yields 0") {
        double position = GENERATE(-10.0, -1.0, -0.5, 0.0, 0.5, 1.0, 5.0, 1.0);
        REQUIRE(Index::index_op(position, 0) == 0);
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
                auto phase = Phase(static_cast<double>(i) / static_cast<double>(size));

                REQUIRE(Index::index_op(phase.get(), size) == i);
            }
        }

        SECTION("Phase-like values") {
            for (std::size_t i = 0; i < size - 2; ++i) {
                auto phase_like = static_cast<double>(i) / static_cast<double>(size);
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
            auto cursor = static_cast<double>(i) / static_cast<double>(size);
            CAPTURE(i, cursor);
            REQUIRE(Index::index_op(cursor, size) == i);
        }
    }

    SECTION("Negative indices") {
        // std::size_t size = GENERATE(10, 100, 1000, 1e4, 1e5, 1e6);
    }

}


TEST_CASE("Index: index_op (negative indices)", "[index]") {
    SECTION("Position is floored, not rounded") {
        std::size_t size = 10;
        REQUIRE(Index::index_op(-1.0, size) == -10);
        REQUIRE(Index::index_op(-0.01, size) == -1);
        REQUIRE(Index::index_op(-0.1 + EPSILON, size) == -1);
        REQUIRE(Index::index_op(-0.49, size) == -5);
        REQUIRE(Index::index_op(-0.5 + EPSILON, size) == -5);
        REQUIRE(Index::index_op(-0.51 + EPSILON, size) == -6);
        REQUIRE(Index::index_op(-0.99, size) == -10);
    }
}


TEST_CASE("Index: index_op under MaxMSP-related constraints", "[index]") {
    // MaxMSP uses epsilon 1e-6 when passing data between objects,
    // which means that Phase::max() and Phase::wrap_around() maps to the same value

    auto new_epsilon = 1e-8;
    override_epsilon(new_epsilon);

    auto phase_maximum = 1 - new_epsilon;

    std::size_t max_size = 1e6;
    for (std::size_t size = 2; size < max_size; ++size) {
        CAPTURE(size);
        REQUIRE(Index::index_op(phase_maximum, size) == size - 1);
    }

}


TEST_CASE("Index: Conversion from Facet", "[index]") {
    std::size_t max = 1e7;
    for (std::size_t i = 0; i < max; ++i) {
        auto closest_double = static_cast<double>(i);
        REQUIRE(Index::from_index_facet(closest_double).get_raw() == i);
    }
}


TEST_CASE("Index: phase_op", "[index]") {
    std::size_t size = 10;
    REQUIRE_THAT(Index::phase_op(0, size), Catch::Matchers::WithinAbs(0.0, 1e-8));
    REQUIRE_THAT(Index::phase_op(1, size), Catch::Matchers::WithinAbs(0.1, 1e-8));
    REQUIRE_THAT(Index::phase_op(5, size), Catch::Matchers::WithinAbs(0.5, 1e-8));
    REQUIRE_THAT(Index::phase_op(9, size), Catch::Matchers::WithinAbs(0.9, 1e-8));


    REQUIRE_THAT(Index::phase_op(-1, size), Catch::Matchers::WithinAbs(-0.1, 1e-8));
    REQUIRE_THAT(Index::phase_op(-10, size), Catch::Matchers::WithinAbs(-1.0, 1e-8));

    REQUIRE_THAT(Index::phase_op(10, size), Catch::Matchers::WithinAbs(1.0, 1e-8));
    REQUIRE_THAT(Index::phase_op(100, size), Catch::Matchers::WithinAbs(10.0, 1e-8));
}


TEST_CASE("Index: phase_op => index op round trip yields initial value", "[index]") {

    SECTION("limits") {
        std::size_t size = GENERATE(1, 10, 100, 1000, 1e4, 1e5, 1e6, 1e7);
        CAPTURE(size);

        REQUIRE(Index::index_op(Index::phase_op(0, size), size) == 0);
        REQUIRE(Index::index_op(Index::phase_op(size - 1, size), size) == size - 1);
    }

    SECTION("Intermediate steps do not yield rounding errors up") {
        std::size_t size = GENERATE(1, 10, 100, 1000, 1e4, 1e5, 1e6);
        CAPTURE(size);

        for (std::size_t i = 0; i < size; ++i) {
            REQUIRE(Index::index_op(Index::phase_op(i, size), size) == i);
        }
    }
}


TEST_CASE("Index: quantize", "[index]") {
    REQUIRE_THAT(Index::quantize(0.0, 4), Catch::Matchers::WithinAbs(0.0, 1e-8));
    REQUIRE_THAT(Index::quantize(0.25 - EPSILON/2, 4), Catch::Matchers::WithinAbs(0.25, 1e-8));
    REQUIRE_THAT(Index::quantize(0.25, 4), Catch::Matchers::WithinAbs(0.25, 1e-8));
    REQUIRE_THAT(Index::quantize(0.25 + EPSILON, 4), Catch::Matchers::WithinAbs(0.25, 1e-8));
    REQUIRE_THAT(Index::quantize(Phase::max(), 4), Catch::Matchers::WithinAbs(0.75, 1e-8));
    REQUIRE_THAT(Index::quantize(1.0, 4), Catch::Matchers::WithinAbs(1.0, 1e-8));
}