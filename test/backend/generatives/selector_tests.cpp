#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/generatives/selector.h"


TEST_CASE("Selector - All Strategy") {
    Selector<int> selector;
    Vec chord = {1, 2, 3, 4, 5};
    auto strategy = SelectionStrategy(SelectionStrategy::All{});
    auto result = selector.process(chord, strategy);
    REQUIRE(result == chord);
}

TEST_CASE("Selector - None Strategy") {
    Selector<int> selector;
    Vec corpus = {1, 2, 3, 4, 5};
    auto strategy = SelectionStrategy(SelectionStrategy::None{});
    auto result = selector.process(corpus, strategy);
    REQUIRE(result.empty());
}

TEST_CASE("Selector - Nth Strategy") {
    Selector<int> selector;
    Vec chord = {1, 2, 3, 4, 5};

    SECTION("Single positive value within bounds") {
        auto strategy = SelectionStrategy(SelectionStrategy::Nth{{0}});
        auto result = selector.process(chord, strategy);
        REQUIRE(result == Vec{1});
    }

    SECTION("Single negative value within bounds") {
        auto strategy = SelectionStrategy(SelectionStrategy::Nth{{-1}});
        auto result = selector.process(chord, strategy);
        REQUIRE(result == Vec{5});
    }

    SECTION("Single value out of bounds") {
        auto strategy = SelectionStrategy(SelectionStrategy::Nth{{5}});
        auto result = selector.process(chord, strategy);
        REQUIRE(result.empty());
    }

    SECTION("Mixed values") {
        auto strategy = SelectionStrategy(SelectionStrategy::Nth{{0, -2, -5, 8, 20, -100}});
        auto result = selector.process(chord, strategy);
        REQUIRE(result == Vec{1, 4});
    }
}

// TODO: Test random

TEST_CASE("SelectorWrapper") {
    SelectorWrapper<Facet, int> selector;
}

