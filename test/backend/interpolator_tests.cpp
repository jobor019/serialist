//#include <iostream>
//#include <catch2/catch_test_macros.hpp>
//#include <catch2/matchers/catch_matchers_floating_point.hpp>
//
//#include "../src/mapping.h"
//#include "../src/interpolator.h"
//
//
//TEST_CASE("Test ContinueInterpolation with MapElement<int>") {
//    Mapping<int> mapping{{  0, 2}
//                         , {4}
//                         , {5}
//                         , {7, 9, 11}};
//    ContinueInterpolation<int> interpolator(12);
//
//    SECTION("Test get with position 0.0") {
//        auto result = interpolator.get(0.0, &mapping);
//        REQUIRE(result.size() == 2);
//        REQUIRE(result[0] == 0);
//        REQUIRE(result[1] == 2);
//    }
//
//    SECTION("Test get with position 1.0") {
//        auto result = interpolator.get(1.0, &mapping);
//        REQUIRE(result.size() == 2);
//        REQUIRE(result[0] == 12);
//        REQUIRE(result[1] == 14);
//    }
//
//    SECTION("Test get with position 0.5") {
//        auto result = interpolator.get(0.5, &mapping);
//        REQUIRE(result.size() == 1);
//        REQUIRE(result[0] == 5);
//    }
//
//    SECTION("Test get with position 2.5") {
//        auto result = interpolator.get(2.5, &mapping);
//        REQUIRE(result.size() == 1);
//        REQUIRE(result[0] == 5 + 12 * 2);
//    }
//
//    SECTION("Test get with position -1.5") {
//        auto result = interpolator.get(-1.5, &mapping);
//        REQUIRE(result.size() == 1);
//        REQUIRE(result[0] == 5 - 12 * 2);
//    }
//
//    SECTION("Test get with empty mapping") {
//        Mapping<int> empty_mapping{};
//        auto result = interpolator.get(2.25, &empty_mapping);
//        REQUIRE(result.empty());
//    }
//
//    SECTION("Immutability") {
//        auto result = interpolator.get(0, &mapping);
//        REQUIRE(result.size() == 2);
//        result.at(0) = -1;
//        result.push_back(-2);
//        REQUIRE(mapping.at(0) == std::vector<int>{0, 2});
//    }
//}
//
//
//// ==============================================================================================
//
//TEST_CASE("Test ModuloInterpolation with MapElement<int>") {
//    Mapping<int> mapping{{  0, 2}
//                         , {4}
//                         , {5}
//                         , {7, 9, 11}};
//    ModuloInterpolation<int> interpolator;
//
//    SECTION("Test get with position 0.0") {
//        auto result = interpolator.get(0.0, &mapping);
//        REQUIRE(result.size() == 2);
//        REQUIRE(result[0] == 0);
//        REQUIRE(result[1] == 2);
//    }
//
//    SECTION("Test get with position 1.0") {
//        auto result = interpolator.get(1.0, &mapping);
//        REQUIRE(result.size() == 2);
//        REQUIRE(result[0] == 0);
//        REQUIRE(result[1] == 2);
//    }
//
//    SECTION("Test get with position 0.5") {
//        auto result = interpolator.get(0.5, &mapping);
//        REQUIRE(result.size() == 1);
//        REQUIRE(result[0] == 5);
//    }
//
//    SECTION("Test get with position 2.5") {
//        auto result = interpolator.get(2.5, &mapping);
//        REQUIRE(result.size() == 1);
//        REQUIRE(result[0] == 5);
//    }
//
//    SECTION("Test get with position -1.5") {
//        auto result = interpolator.get(-1.5, &mapping);
//        REQUIRE(result.size() == 1);
//        REQUIRE(result[0] == 5);
//    }
//
//    SECTION("Test get with empty mapping") {
//        Mapping<int> empty_mapping{};
//        auto result = interpolator.get(2.25, &empty_mapping);
//        REQUIRE(result.empty());
//    }
//
//    SECTION("Immutability") {
//        auto result = interpolator.get(0, &mapping);
//        REQUIRE(result.size() == 2);
//        result.at(0) = -1;
//        result.push_back(-2);
//        REQUIRE(mapping.at(0) == std::vector<int>{0, 2});
//    }
//}
//
//
//// ==============================================================================================
//
//TEST_CASE("Test ClipInterpolation with MapElement<int>") {
//    Mapping<int> mapping{{  0, 2}
//                         , {4}
//                         , {5}
//                         , {7, 9, 11}};
//    ClipInterpolation<int> interpolator;
//
//    SECTION("Test get with position 0.0") {
//        auto result = interpolator.get(0.0, &mapping);
//        REQUIRE(result.size() == 2);
//        REQUIRE(result[0] == 0);
//        REQUIRE(result[1] == 2);
//    }
//
//    SECTION("Test get with position 1.0") {
//        auto result = interpolator.get(1.0, &mapping);
//        REQUIRE(result.size() == 3);
//        REQUIRE(result[0] == 7);
//        REQUIRE(result[1] == 9);
//        REQUIRE(result[2] == 11);
//    }
//
//    SECTION("Test get with position 0.5") {
//        auto result = interpolator.get(0.5, &mapping);
//        REQUIRE(result.size() == 1);
//        REQUIRE(result[0] == 5);
//    }
//
//    SECTION("Test get with position 2.5") {
//        auto result = interpolator.get(2.5, &mapping);
//        REQUIRE(result.size() == 3);
//        REQUIRE(result[0] == 7);
//        REQUIRE(result[1] == 9);
//        REQUIRE(result[2] == 11);
//    }
//
//    SECTION("Test get with position -1.5") {
//        auto result = interpolator.get(-1.5, &mapping);
//        REQUIRE(result.size() == 2);
//        REQUIRE(result[0] == 0);
//        REQUIRE(result[1] == 2);
//    }
//
//    SECTION("Test get with empty mapping") {
//        Mapping<int> empty_mapping{};
//        auto result = interpolator.get(2.25, &empty_mapping);
//        REQUIRE(result.empty());
//    }
//
//    SECTION("Immutability") {
//        auto result = interpolator.get(0, &mapping);
//        REQUIRE(result.size() == 2);
//        result.at(0) = -1;
//        result.push_back(-2);
//        REQUIRE(mapping.at(0) == std::vector<int>{0, 2});
//    }
//}
//
//
//// ==============================================================================================
//
//TEST_CASE("Test PassInterpolation with MapElement<int>") {
//    Mapping<int> mapping{{  0, 2}
//                         , {4}
//                         , {5}
//                         , {7, 9, 11}};
//    PassInterpolation<int> interpolator;
//
//    SECTION("Test get with position 0.0") {
//        auto result = interpolator.get(0.0, &mapping);
//        REQUIRE(result.size() == 2);
//        REQUIRE(result[0] == 0);
//        REQUIRE(result[1] == 2);
//    }
//
//    SECTION("Test get with position 1.0") {
//        auto result = interpolator.get(1.0, &mapping);
//        REQUIRE(result.empty());
//    }
//
//    SECTION("Test get with position 0.5") {
//        auto result = interpolator.get(0.5, &mapping);
//        REQUIRE(result.size() == 1);
//        REQUIRE(result[0] == 5);
//    }
//
//    SECTION("Test get with position 2.5") {
//        auto result = interpolator.get(2.5, &mapping);
//        REQUIRE(result.empty());
//    }
//
//    SECTION("Test get with position -1.5") {
//        auto result = interpolator.get(-1.5, &mapping);
//        REQUIRE(result.empty());
//    }
//
//    SECTION("Test get with empty mapping") {
//        Mapping<int> empty_mapping{};
//        auto result = interpolator.get(2.25, &empty_mapping);
//        REQUIRE(result.empty());
//    }
//
//    SECTION("Immutability") {
//        auto result = interpolator.get(0, &mapping);
//        REQUIRE(result.size() == 2);
//        result.at(0) = -1;
//        result.push_back(-2);
//        REQUIRE(mapping.at(0) == std::vector<int>{0, 2});
//    }
//}
//
//
//// ==============================================================================================
//
//
//TEST_CASE("Interpolation using ContinueInterpolation with sizes between 1 and 1000 ") {
//    for (std::size_t mapping_size = 1; mapping_size <= 1000; ++mapping_size) {
//        Mapping<int> mapping;
//        for (std::size_t i = 0; i < mapping_size; ++i) {
//            mapping.add(MapElement<int>{static_cast<int>(i)});
//        }
//
//        ContinueInterpolation<int> interpolator(static_cast<int>(mapping_size));
//
//        double position = 0;
//        double increment = 1.0 / static_cast<double>(mapping_size);
//        for (std::size_t i = 0; i < mapping_size; ++i) {
//
//            REQUIRE(interpolator.get(position, &mapping) == mapping.at(i));
//            position += increment;
//        }
//    }
//}
//
//
//TEST_CASE("Interpolation using ModuloInterpolation with sizes between 1 and 1000 ") {
//    for (std::size_t mapping_size = 1; mapping_size <= 1000; ++mapping_size) {
//        Mapping<int> mapping;
//        for (std::size_t i = 0; i < mapping_size; ++i) {
//            mapping.add(MapElement<int>{static_cast<int>(i)});
//        }
//
//        ModuloInterpolation<int> interpolator;
//
//        double position = 0;
//        double increment = 1.0 / static_cast<double>(mapping_size);
//        for (std::size_t i = 0; i < mapping_size; ++i) {
//
//            REQUIRE(interpolator.get(position, &mapping) == mapping.at(i));
//            position += increment;
//        }
//    }
//}
//
//
//TEST_CASE("Interpolation using CLipInterpolator with sizes between 1 and 1000 ") {
//    for (std::size_t mapping_size = 1; mapping_size <= 1000; ++mapping_size) {
//        Mapping<int> mapping;
//        for (std::size_t i = 0; i < mapping_size; ++i) {
//            mapping.add(MapElement<int>{static_cast<int>(i)});
//        }
//
//        ClipInterpolation<int> interpolator;
//
//        double position = 0;
//        double increment = 1.0 / static_cast<double>(mapping_size);
//        for (std::size_t i = 0; i < mapping_size; ++i) {
//
//            REQUIRE(interpolator.get(position, &mapping) == mapping.at(i));
//            position += increment;
//        }
//    }
//}
//
//
//TEST_CASE("Interpolation using ClipInterpolation with sizes between 1 and 1000 ") {
//    for (std::size_t mapping_size = 1; mapping_size <= 1000; ++mapping_size) {
//        Mapping<int> mapping;
//        for (std::size_t i = 0; i < mapping_size; ++i) {
//            mapping.add(MapElement<int>{static_cast<int>(i)});
//        }
//
//        PassInterpolation<int> interpolator;
//
//        double position = 0;
//        double increment = 1.0 / static_cast<double>(mapping_size);
//        for (std::size_t i = 0; i < mapping_size; ++i) {
//
//            REQUIRE(interpolator.get(position, &mapping) == mapping.at(i));
//            position += increment;
//        }
//    }
//}
//
//
//
//
//
//
