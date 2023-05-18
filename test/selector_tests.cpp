#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "../src/selector.h"


TEST_CASE("Test IdentitySelection class ") {
    std::vector<int> empty;
    std::vector<int> single{2};
    std::vector<int> multi{0, 2, 5};

    IdentitySelection<int> selector;

    REQUIRE(selector.get(empty).empty());
    REQUIRE(selector.get(single) == std::vector<int>{2});
    REQUIRE(selector.get(multi) == std::vector<int>{0, 2, 5});

    // check mutability
    auto e = selector.get(single);
    e[0] += 1;
    REQUIRE(single == std::vector<int>{2});
}


// ==============================================================================================

TEST_CASE("Test NthSelection class") {
    std::vector<int> empty;
    std::vector<int> single{2};
    std::vector<int> multi{0, 2, 5};

    SECTION("Valid index") {
        NthSelection<int> selector(1);
        REQUIRE(selector.get(multi) == std::vector<int>{2});
    }

    SECTION("Negative index") {
        NthSelection<int> selector(-1);
        REQUIRE(selector.get(multi) == std::vector<int>{5});
    }

    SECTION("Index out of range") {
        NthSelection<int> selector(5);
        REQUIRE(selector.get(multi).empty());
    }
}


// ==============================================================================================

TEST_CASE("Test FirstSelector class") {
    std::vector<int> empty;
    std::vector<int> single{2};
    std::vector<int> multi{0, 2, 5};

    FirstSelector<int> selector;

    REQUIRE(selector.get(empty).empty());
    REQUIRE(selector.get(single) == std::vector<int>{2});
    REQUIRE(selector.get(multi) == std::vector<int>{0});

    // check mutability
    auto e = selector.get(single);
    e[0] += 1;
    REQUIRE(single == std::vector<int>{2});
}

