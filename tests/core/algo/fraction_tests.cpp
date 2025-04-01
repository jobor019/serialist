
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/policies/policies.h"
#include "core/types/fraction.h"

using namespace serialist;

TEST_CASE("ExtendedFraction::from_decimal") {
    SECTION("Rounded from 0.999") {
        auto allowed_denoms = Vec<long>{1, 2, 3, 4, 5, 7, 8};
        auto q = ExtendedFraction::from_decimal(0.999, allowed_denoms); // Expected: {1, 0/1}
        REQUIRE(q.get_integral_part() == 1);
        REQUIRE(q.get_n() == 0);
        REQUIRE(q.get_d() == 1);
    }

    SECTION("Rounded from 0.999 with different allowed_denoms") {
        auto allowed_denoms = Vec<long>{3, 4, 5, 7, 8};
        auto q = ExtendedFraction::from_decimal(0.999, allowed_denoms); // Expected: {1, 0/1}
        REQUIRE(q.get_integral_part() == 1);
        REQUIRE(q.get_n() == 0);
        REQUIRE(q.get_d() == 3);
    }

    SECTION("Negative numbers") {
        auto allowed_denoms = Vec<long>{1, 2, 3, 4, 5, 7, 8};
        auto q = ExtendedFraction::from_decimal(-1.75, allowed_denoms); // Expected: {-1, -3/4}
        REQUIRE(q.get_integral_part() == -1);
        REQUIRE(q.get_n() == -3);
        REQUIRE(q.get_d() == 4);
    }
}


TEST_CASE("ExtendedFraction::to_string (mixed)") {
    SECTION("Zero") {
        auto q = ExtendedFraction{0, 0, 1};
        auto s = q.to_string();
        REQUIRE(s == "0");
    }

    SECTION("One") {
        auto q = ExtendedFraction{1,0, 1};
        auto s = q.to_string();
        REQUIRE(s == "1");
    }


    SECTION("Positive number less than 1") {
        auto q = ExtendedFraction{0, 3, 4};
        auto s = q.to_string();
        REQUIRE(s == "3/4");
    }

    SECTION("Positive number less than 1 with sign") {
        auto q = ExtendedFraction{0, 3, 4};
        auto s = q.to_string(ExtendedFraction::Format::mixed, true);
        REQUIRE(s == "+3/4");
    }

    SECTION("Positive number greater than 1 with fractional part") {
        auto q = ExtendedFraction{1, 2, 3};
        auto s = q.to_string();
        REQUIRE(s == "1 2/3");
    }

    SECTION("Positive number greater than 1 without fractional part") {
        auto q = ExtendedFraction{2, 0, 3};
        auto s = q.to_string();
        REQUIRE(s == "2 0/3");
    }

    SECTION("Negative number -1") {
        auto q = ExtendedFraction{-1, 0, 1};
        auto s = q.to_string();
        REQUIRE(s == "-1");
    }

    SECTION("Negative number between -1 and 0") {
        auto q = ExtendedFraction{0, -2, 3};
        auto s = q.to_string();
        REQUIRE(s == "-2/3");
    }

    SECTION("Negative number less than -1") {
        auto q = ExtendedFraction{-1, -2, 3};
        auto s = q.to_string();
        REQUIRE(s == "-1 2/3");
    }
}