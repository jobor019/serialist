//#include <iostream>
//#include <catch2/catch_test_macros.hpp>
//#include <catch2/matchers/catch_matchers_floating_point.hpp>
//
//#include "../src/mapping.h"
//#include "../src/looper.h"
//
//
//TEST_CASE("Test Looper class") {
//    Mapping<int> mapping{{  0, 2}
//                         , {4}
//                         , {5}
//                         , {7, 9, 11}};
//
//    Looper<int> looper{mapping, 1.0, 0.0, Phasor::Mode::stepped, std::make_unique<FirstAccessor<long>>(), nullptr, nullptr};
//
//    SECTION("Test get with position 0.0") {
//        auto result = interpolator.get(0.0, mapping);
//        REQUIRE(result.size() == 2);
//        REQUIRE(result[0] == 0);
//        REQUIRE(result[1] == 2);
//    }
//
//    SECTION("Test get with position 1.0") {
//        auto result = interpolator.get(1.0, mapping);
//        REQUIRE(result.size() == 2);
//        REQUIRE(result[0] == 12);
//        REQUIRE(result[1] == 14);
//    }
//
//    SECTION("Test get with position 0.5") {
//        auto result = interpolator.get(0.5, mapping);
//        REQUIRE(result.size() == 1);
//        REQUIRE(result[0] == 5);
//    }
//
//    SECTION("Test get with position 2.5") {
//        auto result = interpolator.get(2.5, mapping);
//        REQUIRE(result.size() == 1);
//        REQUIRE(result[0] == 5 + 12 * 2);
//    }
//
//    SECTION("Test get with position -1.5") {
//        auto result = interpolator.get(-1.5, mapping);
//        REQUIRE(result.size() == 1);
//        REQUIRE(result[0] == 5 - 12 * 2);
//    }
//
//    SECTION("Test get with empty mapping") {
//        Mapping<int> empty_mapping{};
//        auto result = interpolator.get(2.25, empty_mapping);
//        REQUIRE(result.empty());
//    }
//}
