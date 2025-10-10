
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <iostream>
#include "core/types/facet.h"

using namespace serialist;

TEST_CASE("Boolean conversion", "[facet]") {
    // operator== true results
    auto t = true;
    auto facet_true = Facet(t);
    REQUIRE(facet_true == t);

    auto f = false;
    auto facet_false = Facet(f);
    REQUIRE(facet_false == f);

    // operator== false results
    REQUIRE(!(facet_true == f));
    REQUIRE(!(facet_false == t));

    // operator!= true results
    REQUIRE((facet_true != f));
    REQUIRE((facet_false != t));

    // operator!= false results
    REQUIRE(!(facet_true != t));
    REQUIRE(!(facet_false != f));

    // integer & double conversion in ranges integral ranges
    for (int i = 1; i < 10000; ++i) {
        auto facet_int = Facet(i);
        auto facet_double = Facet(static_cast<double>(i));
        REQUIRE(static_cast<bool>(facet_int));
        REQUIRE(static_cast<bool>(facet_double));
    }

    for (int i = 0; i > -10000; --i) {
        auto facet_int = Facet(i);
        auto facet_double = Facet(static_cast<double>(i));
        REQUIRE(!static_cast<bool>(facet_int));
        REQUIRE(!static_cast<bool>(facet_double));
    }

    // double conversion (false in range [0..0.5])
    double d = 0.0;
    while (d < 0.5) {
        auto facet = Facet(d);
        REQUIRE(!static_cast<bool>(facet));
        d += 0.00001;
    }

    // double conversion (true in range [0.5..])
    double d2 = 0.5;
    while (d2 < 1.0) {
        auto facet = Facet(d2);
        REQUIRE(static_cast<bool>(facet));
        d2 += 0.00001;
    }
}


TEST_CASE("Integer conversion", "[facet]") {
    // comparisons i1 == i2 positive values
    for (long i = 0; i < 10'000'000; ++i) {
        auto facet = Facet(i);
        REQUIRE(facet == i);
    }

    // comparisons i1 == i2 negative values
    for (long i = 0; i > -10'000'000; --i) {
        auto facet = Facet(i);
        REQUIRE(facet == i);
    }
}

enum class Enum100 {
    e1, e2, e3, e4, e5, e6, e7, e8, e9, e10,
    e11, e12, e13, e14, e15, e16, e17, e18, e19, e20,
    e21, e22, e23, e24, e25, e26, e27, e28, e29, e30,
    e31, e32, e33, e34, e35, e36, e37, e38, e39, e40,
    e41, e42, e43, e44, e45, e46, e47, e48, e49, e50,
    e51, e52, e53, e54, e55, e56, e57, e58, e59, e60,
    e61, e62, e63, e64, e65, e66, e67, e68, e69, e70,
    e71, e72, e73, e74, e75, e76, e77, e78, e79, e80,
    e81, e82, e83, e84, e85, e86, e87, e88, e89, e90,
    e91, e92, e93, e94, e95, e96, e97, e98, e99, e100
};

enum class Enum37 {
    e1, e2, e3, e4, e5, e6, e7, e8, e9, e10,
    e11, e12, e13, e14, e15, e16, e17, e18, e19, e20,
    e21, e22, e23, e24, e25, e26, e27, e28, e29, e30,
    e31, e32, e33, e34, e35, e36, e37
};

enum class Enum19 {
    e1, e2, e3, e4, e5, e6, e7, e8, e9, e10,
    e11, e12, e13, e14, e15, e16, e17, e18, e19
};

enum class Enum11 {
    e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11
};

enum class Enum7 {
    e1, e2, e3, e4, e5, e6, e7
};

enum class Enum3 {
    e1, e2, e3
};

enum class Enum2 {
    e1, e2
};

enum class Enum1 {
    e1
};



template<typename T>
void assert_int_enum_round_trip() {
    auto count = static_cast<int>(magic_enum::enum_count<T>());
    CAPTURE(count);
    REQUIRE(count > 0);
    for (int i = 0; i < count; ++i) {
        auto enum_value = static_cast<T>(i);
        auto facet_cast_value = static_cast<Facet>(enum_value);
        auto facet_ctor_value = Facet(enum_value);
        REQUIRE(static_cast<T>(facet_cast_value) == enum_value);
        REQUIRE(static_cast<T>(facet_ctor_value) == enum_value);
    }
}


TEST_CASE("Facet: Double to enum conversion is discretized correctly", "[facet]") {
    auto eps2 = 2.0 * Facet::ENUM_EPSILON;
    REQUIRE(static_cast<Enum3>(Facet(0.0)) == Enum3::e1);
    REQUIRE(static_cast<Enum3>(Facet(1.0/3.0 - eps2)) == Enum3::e1);
    REQUIRE(static_cast<Enum3>(Facet(1.0/3.0 + eps2)) == Enum3::e2);
    REQUIRE(static_cast<Enum3>(Facet(2.0/3.0 - eps2)) == Enum3::e2);
    REQUIRE(static_cast<Enum3>(Facet(2.0/3.0 + eps2)) == Enum3::e3);
    REQUIRE(static_cast<Enum3>(Facet(1.0)) == Enum3::e3);

    SECTION("Out of bounds values are clamped") {
        REQUIRE(static_cast<Enum3>(Facet(-0.1)) == Enum3::e1);
        REQUIRE(static_cast<Enum3>(Facet(1.1)) == Enum3::e3);
    }
}


TEST_CASE("Facet: Int to enum conversion is discretized correctly", "[facet]") {
    SECTION("Int enum round trip") {
        assert_int_enum_round_trip<Enum100>();
        assert_int_enum_round_trip<Enum37>();
        assert_int_enum_round_trip<Enum19>();
        assert_int_enum_round_trip<Enum11>();
        assert_int_enum_round_trip<Enum7>();
        assert_int_enum_round_trip<Enum3>();
        assert_int_enum_round_trip<Enum2>();
        assert_int_enum_round_trip<Enum1>();
    }
}


TEST_CASE("Arithmetic operators", "[facet]") {
    Facet f1(1.0);
    Facet f2(2.0);
    Facet f3(3.0);
    double d1 = 4.0;
    int i1 = 5;

    SECTION("Addition") {
        REQUIRE(f1 + f2 == Facet(3.0));
        REQUIRE(f1 + 2.0 == Facet(3.0));
        REQUIRE(2.0 + f1 == Facet(3.0));
        REQUIRE(f1 + d1 == Facet(5.0));
        REQUIRE(d1 + f1 == Facet(5.0));
        REQUIRE(f1 + i1 == Facet(6.0));
        REQUIRE(i1 + f1 == Facet(6.0));
        REQUIRE(f1 + f2 + f3 == Facet(6.0));
        REQUIRE(f1 == Facet(1.0));
        REQUIRE(f2 == Facet(2.0));
        REQUIRE(f3 == Facet(3.0));
    }

}

TEST_CASE("Facet comparisons", "[facet]") {
    Facet f1(1.0);
    Facet f2(2.0);
    Facet f3(3.0);
    REQUIRE(f1 < f2);
    REQUIRE(f2 > f1);
    REQUIRE(f1 <= f2);
    REQUIRE(f2 >= f1);
    REQUIRE(f1 <= f3);
    REQUIRE(f3 >= f1);
    REQUIRE(f3 == f3);
    REQUIRE(f3 != f2);
}


//TEST_CASE("Performance") {
//    // Creation
//    std::size_t n = 10'000'000;
//    std::vector<Facet> v1;
//    v1.reserve(n);
//
//    auto t1 = std::chrono::high_resolution_clock::now();
//    for (std::size_t i = 0; i < n; ++i) {
//        v1.emplace_back(i);
//    }
//
//    auto t2 = std::chrono::high_resolution_clock::now();
//
//    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count() << " usec\n";
//
//
//    std::vector<double> v2;
//    v2.reserve(n);
//
//    t1 = std::chrono::high_resolution_clock::now();
//    for (std::size_t i = 0; i < n; ++i) {
//        v2.emplace_back(i);
//    }
//    t2 = std::chrono::high_resolution_clock::now();
//
//    std::cout << std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count() << " usec\n";
//
//
//
//
//}