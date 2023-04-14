#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../src/mapping.h"

TEST_CASE("MapElement class") {
    SECTION("empty") {
        MapElement<int> m{};
        REQUIRE(m.size() == 0);
    }

    SECTION("one elem") {
        MapElement<int> m{123};
        REQUIRE(m.at(0) == 123);
    }

    SECTION("several") {
        MapElement<int> m{123, 234, 888};
        REQUIRE(m.at(0) == 123);
        REQUIRE(m.at(1) == 234);
        REQUIRE(m.at(2) == 888);
        REQUIRE(m.size() == 3);
    }

    SECTION("multi") {
        MapElement<std::vector<int>> m{{  1, 2, 3}
                                       , {4, 5, 6}
                                       , {7, 8, 9}};
        REQUIRE(m.at(0) == std::vector{1, 2, 3});
        REQUIRE(m.at(1) == std::vector{4, 5, 6});
        REQUIRE(m.size() == 3);
    }
}


TEST_CASE("Test Mapping class") {
    SECTION("Test default constructor") {
        Mapping<int> mapping;
        REQUIRE(mapping.size() == 0);
        REQUIRE(mapping.empty());
    }

    SECTION("Test initializer list constructor with MapElement") {
        Mapping<int> mapping{{  1, 2, 3}
                             , {4, 5}
                             , {6}};
        REQUIRE(mapping.size() == 3);
        REQUIRE(mapping.at(0).size() == 3);
        REQUIRE(mapping.at(1).size() == 2);
        REQUIRE(mapping.at(2).size() == 1);
        REQUIRE_THROWS_AS(mapping.at(3), std::out_of_range);
    }

    SECTION("Test initializer list constructor with T") {
        Mapping<int> mapping{1, 2, 3};
        REQUIRE(mapping.size() == 3);
        REQUIRE(mapping.at(0).size() == 1);
        REQUIRE(mapping.at(1).size() == 1);
        REQUIRE(mapping.at(2).size() == 1);
    }

    SECTION("Test vector constructor") {
        std::vector<int> values{1, 2, 3};
        Mapping<int> mapping(values);
        REQUIRE(mapping.size() == 3);
        REQUIRE(mapping.at(0).size() == 1);
        REQUIRE(mapping.at(1).size() == 1);
        REQUIRE(mapping.at(2).size() == 1);
    }

    SECTION("Test add with MapElement") {
        Mapping<int> mapping{{  1, 2}
                             , {4}};
        mapping.add({3}, 1);
        REQUIRE(mapping.size() == 3);
        REQUIRE(mapping.at(0) == MapElement<int>{1, 2});
        REQUIRE(mapping.at(1) == MapElement<int>{3});
        REQUIRE(mapping.at(2) == MapElement<int>{4});
    }

    SECTION("Test add with vector of MapElement") {
        Mapping<int> mapping{{  1, 2}
                             , {4}};
        mapping.add({{  3, 5}
                     , {6}});
        REQUIRE(mapping.size() == 4);
        REQUIRE(mapping.at(0) == MapElement<int>{1, 2});
        REQUIRE(mapping.at(1) == MapElement<int>{4});
        REQUIRE(mapping.at(2) == MapElement<int>{3, 5});
        REQUIRE(mapping.at(3) == MapElement<int>{6});
    }

    SECTION("Test size and empty methods") {
        Mapping<int> mapping{{  1, 2}
                             , {4}};
        REQUIRE(mapping.size() == 2);
        REQUIRE_FALSE(mapping.empty());
        mapping.add({3});
        REQUIRE(mapping.size() == 3);
        REQUIRE_FALSE(mapping.empty());
        mapping.add({MapElement<int>{1}, MapElement<int>{2}});
        REQUIRE(mapping.size() == 5);
        REQUIRE_FALSE(mapping.empty());
    }

    SECTION("Test add with negative index") {
        Mapping<int> mapping{{  1, 2}
                             , {3, 4}};
        mapping.add({5}, -1);
        REQUIRE(mapping.size() == 3);
        REQUIRE(mapping.at(0) == MapElement<int>{1, 2});
        REQUIRE(mapping.at(1) == MapElement<int>{3, 4});
        REQUIRE(mapping.at(2) == MapElement<int>{5});
    }

    SECTION("Test add with out of bounds index") {
        Mapping<int> mapping{{  1, 2}
                             , {3, 4}};
        mapping.add({5}, 10);
        REQUIRE(mapping.size() == 3);
        REQUIRE(mapping.at(0) == MapElement<int>{1, 2});
        REQUIRE(mapping.at(1) == MapElement<int>{3, 4});
        REQUIRE(mapping.at(2) == MapElement<int>{5});
    }

    SECTION("Test add with empty MapElement") {
        Mapping<int> mapping{{  1, 2}
                             , {3}
                             , {4}};
        mapping.add(MapElement<int>{}, 1);
        REQUIRE(mapping.size() == 4);
        REQUIRE(mapping.at(0) == MapElement<int>{1, 2});
        REQUIRE(mapping.at(1).empty());
        REQUIRE(mapping.at(2) == MapElement<int>{3});
    }

    SECTION("Test add with negative index") {
        Mapping<int> mapping{{  1, 2}
                             , {4}};
        mapping.add({3}, -2);
        REQUIRE(mapping.size() == 3);
        REQUIRE(mapping.at(0) == MapElement<int>{1, 2});
        REQUIRE(mapping.at(1) == MapElement<int>{3});
        REQUIRE(mapping.at(2) == MapElement<int>{4});
    }

    SECTION("Test add with index greater than size") {
        Mapping<int> mapping{{1, 2}, {3}};
        mapping.add({4, 5}, 10);
        REQUIRE(mapping.size() == 3);
        REQUIRE(mapping.at(0) == MapElement<int>{1, 2});
        REQUIRE(mapping.at(1) == MapElement<int>{3});
        REQUIRE(mapping.at(2) == MapElement<int>{4, 5});
    }

    SECTION("Test add with multiple MapElement elements") {
        Mapping<int> mapping{{1, 2}, {4}};
        mapping.add({{  3}
                     , {5, 6}}, 1);
        REQUIRE(mapping.size() == 4);
        REQUIRE(mapping.at(0) == MapElement<int>{1, 2});
        REQUIRE(mapping.at(1) == MapElement<int>{3});
        REQUIRE(mapping.at(2) == MapElement<int>{5, 6});
        REQUIRE(mapping.at(3) == MapElement<int>{4});
    }

    SECTION("Test add with move semantics") {
        Mapping<std::string> mapping{MapElement<std::string>{"a", "b"}
                                     , MapElement<std::string>{"c", "d"}};
        auto element = MapElement<std::string>{"e", "f"};
        mapping.add(std::move(element), 1);
        REQUIRE(mapping.size() == 3);
        REQUIRE(mapping.at(0) == MapElement<std::string>{"a", "b"});
        REQUIRE(mapping.at(1) == MapElement<std::string>{"e", "f"});
        REQUIRE(mapping.at(2) == MapElement<std::string>{"c", "d"});
    }

    SECTION("Test add with multiple MapElement elements and move semantics") {
        Mapping<std::string> mapping{MapElement<std::string>{"a", "b"}
                                     , MapElement<std::string>{"c", "d"}};
        auto element1 = MapElement<std::string>{"e", "f"};
        auto element2 = MapElement<std::string>{"g", "h"};
        mapping.add({std::move(element1), std::move(element2)}, 1);
        REQUIRE(mapping.size() == 4);
        REQUIRE(mapping.at(0) == MapElement<std::string>{"a", "b"});
        REQUIRE(mapping.at(1) == MapElement<std::string>{"e", "f"});
        REQUIRE(mapping.at(2) == MapElement<std::string>{"g", "h"});
        REQUIRE(mapping.at(3) == MapElement<std::string>{"c", "d"});
    }

}
