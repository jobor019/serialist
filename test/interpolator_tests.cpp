#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../src/mapping.h"
#include "../src/interpolator.h"
#include "../src/selector.h"


TEST_CASE("Test ContinueInterpolator with MapElement<int>") {
    Mapping<int> mapping{{  0, 2}
                         , {4}
                         , {5}
                         , {7, 9, 11}};
    ContinueInterpolator<int> interpolator(12);

    SECTION("Test get with position 0.0") {
        auto result = interpolator.get(0.0, mapping);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 2);
    }

    SECTION("Test get with position 1.0") {
        auto result = interpolator.get(1.0, mapping);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 12);
        REQUIRE(result[1] == 14);
    }

    SECTION("Test get with position 0.5") {
        auto result = interpolator.get(0.5, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with position 2.5") {
        auto result = interpolator.get(2.5, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5 + 12 * 2);
    }

    SECTION("Test get with position -1.5") {
        auto result = interpolator.get(-1.5, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5 - 12 * 2);
    }

    SECTION("Test get with empty mapping") {
        Mapping<int> empty_mapping{};
        auto result = interpolator.get(2.25, empty_mapping);
        REQUIRE(result.empty());
    }
}


// ==============================================================================================

TEST_CASE("Test ModuloInterpolator with MapElement<int>") {
    Mapping<int> mapping{{  0, 2}
                         , {4}
                         , {5}
                         , {7, 9, 11}};
    ModuloInterpolator<int> interpolator;

    SECTION("Test get with position 0.0") {
        auto result = interpolator.get(0.0, mapping);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 2);
    }

    SECTION("Test get with position 1.0") {
        auto result = interpolator.get(1.0, mapping);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 2);
    }

    SECTION("Test get with position 0.5") {
        auto result = interpolator.get(0.5, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with position 2.5") {
        auto result = interpolator.get(2.5, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with position -1.5") {
        auto result = interpolator.get(-1.5, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with empty mapping") {
        Mapping<int> empty_mapping{};
        auto result = interpolator.get(2.25, empty_mapping);
        REQUIRE(result.empty());
    }
}


// ==============================================================================================

TEST_CASE("Test ClipInterpolator with MapElement<int>") {
    Mapping<int> mapping{{  0, 2}
                         , {4}
                         , {5}
                         , {7, 9, 11}};
    ClipInterpolator<int> interpolator;

    SECTION("Test get with position 0.0") {
        auto result = interpolator.get(0.0, mapping);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 2);
    }

    SECTION("Test get with position 1.0") {
        auto result = interpolator.get(1.0, mapping);
        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == 7);
        REQUIRE(result[1] == 9);
        REQUIRE(result[2] == 11);
    }

    SECTION("Test get with position 0.5") {
        auto result = interpolator.get(0.5, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with position 2.5") {
        auto result = interpolator.get(2.5, mapping);
        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == 7);
        REQUIRE(result[1] == 9);
        REQUIRE(result[2] == 11);
    }

    SECTION("Test get with position -1.5") {
        auto result = interpolator.get(-1.5, mapping);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 2);
    }

    SECTION("Test get with empty mapping") {
        Mapping<int> empty_mapping{};
        auto result = interpolator.get(2.25, empty_mapping);
        REQUIRE(result.empty());
    }
}


// ==============================================================================================

TEST_CASE("Test PassInterpolator with MapElement<int>") {
    Mapping<int> mapping{{  0, 2}
                         , {4}
                         , {5}
                         , {7, 9, 11}};
    PassInterpolator<int> interpolator;

    SECTION("Test get with position 0.0") {
        auto result = interpolator.get(0.0, mapping);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 2);
    }

    SECTION("Test get with position 1.0") {
        auto result = interpolator.get(1.0, mapping);
        REQUIRE(result.empty());
    }

    SECTION("Test get with position 0.5") {
        auto result = interpolator.get(0.5, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with position 2.5") {
        auto result = interpolator.get(2.5, mapping);
        REQUIRE(result.empty());
    }

    SECTION("Test get with position -1.5") {
        auto result = interpolator.get(-1.5, mapping);
        REQUIRE(result.empty());
    }

    SECTION("Test get with empty mapping") {
        Mapping<int> empty_mapping{};
        auto result = interpolator.get(2.25, empty_mapping);
        REQUIRE(result.empty());
    }
}


// ==============================================================================================


TEST_CASE("Interpolation using ContinueInterpolator with sizes between 1 and 1000 ") {
    for (std::size_t mapping_size = 1; mapping_size <= 1000; ++mapping_size) {
        Mapping<int> mapping;
        for (std::size_t i = 0; i < mapping_size; ++i) {
            mapping.add(MapElement<int>{static_cast<int>(i)});
        }

        ContinueInterpolator<int> interpolator(static_cast<int>(mapping_size));

        double position = 0;
        double increment = 1.0 / static_cast<double>(mapping_size);
        for (std::size_t i = 0; i < mapping_size; ++i) {

            REQUIRE(interpolator.get(position, mapping) == mapping.at(i));
            position += increment;
        }
    }
}


TEST_CASE("Interpolation using ModuloInterpolator with sizes between 1 and 1000 ") {
    for (std::size_t mapping_size = 1; mapping_size <= 1000; ++mapping_size) {
        Mapping<int> mapping;
        for (std::size_t i = 0; i < mapping_size; ++i) {
            mapping.add(MapElement<int>{static_cast<int>(i)});
        }

        ModuloInterpolator<int> interpolator;

        double position = 0;
        double increment = 1.0 / static_cast<double>(mapping_size);
        for (std::size_t i = 0; i < mapping_size; ++i) {

            REQUIRE(interpolator.get(position, mapping) == mapping.at(i));
            position += increment;
        }
    }
}


TEST_CASE("Interpolation using CLipInterpolator with sizes between 1 and 1000 ") {
    for (std::size_t mapping_size = 1; mapping_size <= 1000; ++mapping_size) {
        Mapping<int> mapping;
        for (std::size_t i = 0; i < mapping_size; ++i) {
            mapping.add(MapElement<int>{static_cast<int>(i)});
        }

        ClipInterpolator<int> interpolator;

        double position = 0;
        double increment = 1.0 / static_cast<double>(mapping_size);
        for (std::size_t i = 0; i < mapping_size; ++i) {

            REQUIRE(interpolator.get(position, mapping) == mapping.at(i));
            position += increment;
        }
    }
}


TEST_CASE("Interpolation using ClipInterpolator with sizes between 1 and 1000 ") {
    for (std::size_t mapping_size = 1; mapping_size <= 1000; ++mapping_size) {
        Mapping<int> mapping;
        for (std::size_t i = 0; i < mapping_size; ++i) {
            mapping.add(MapElement<int>{static_cast<int>(i)});
        }

        PassInterpolator<int> interpolator;

        double position = 0;
        double increment = 1.0 / static_cast<double>(mapping_size);
        for (std::size_t i = 0; i < mapping_size; ++i) {

            REQUIRE(interpolator.get(position, mapping) == mapping.at(i));
            position += increment;
        }
    }
}






