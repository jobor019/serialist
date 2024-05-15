#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/generatives/selector.h"
#include "time_point.h"


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

TEST_CASE("SelectorAdapter") {
    ParameterHandler handler;
    Variable<Facet, double> strategy_type("", handler, 0.0);
    Variable<Facet, std::size_t> nth_index("", handler, 0);
    Variable<Facet, std::size_t> n_random("", handler, 1);
    SelectorAdapter strategy(SelectorAdapter::CLASS_NAME, handler, &strategy_type, &nth_index, &n_random);

    auto num_strategies = SelectionStrategy::count();
    auto current_strategy = 0.0;
    auto increment = 1.0 / static_cast<double>(num_strategies);

    strategy.update_time(TimePoint());
    auto result = strategy.process();
    REQUIRE(!result.is_empty_like());
    REQUIRE(result.first().value().is<SelectionStrategy::All>());

    current_strategy += increment;
    strategy_type.set_value(current_strategy);
    strategy.update_time(TimePoint());
    result = strategy.process();
    REQUIRE(!result.is_empty_like());
    REQUIRE(result.first().value().is<SelectionStrategy::None>());

    current_strategy += increment;
    strategy_type.set_value(current_strategy);
    strategy.update_time(TimePoint());
    result = strategy.process();
    REQUIRE(!result.is_empty_like());
    REQUIRE(result.first().value().is<SelectionStrategy::Nth>());

    current_strategy += increment;
    strategy_type.set_value(current_strategy);
    strategy.update_time(TimePoint());
    result = strategy.process();
    REQUIRE(!result.is_empty_like());
    REQUIRE(result.first().value().is<SelectionStrategy::Randomize>());
}

