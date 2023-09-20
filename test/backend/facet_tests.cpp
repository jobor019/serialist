
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <iostream>
#include "core/facet.h"


TEST_CASE("Boolean conversion") {
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


TEST_CASE("Integer conversion") {
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
void assert_enum() {
    auto count = static_cast<int>(magic_enum::enum_count<T>());
    REQUIRE(count > 0);
    for (int i = 0; i < count; ++i) {
        auto e = static_cast<T>(i);
        auto f1 = static_cast<Facet>(e);
        auto f2 = Facet(e);
        REQUIRE(static_cast<T>(f1) == e);
        REQUIRE(static_cast<T>(f2) == e);
    }
}


TEST_CASE("Enum conversion") {
    // True assertions
    assert_enum<Enum100>();
    assert_enum<Enum37>();
    assert_enum<Enum19>();
    assert_enum<Enum11>();
    assert_enum<Enum7>();
    assert_enum<Enum3>();
    assert_enum<Enum2>();
    assert_enum<Enum1>();


    // False assertions
    auto max_enum = Enum100::e100;
    auto prev_enum =static_cast<Enum100>(0);
    for (int i = 1; i < static_cast<int>(max_enum); ++i) {
        auto f = Facet(prev_enum);
        prev_enum = static_cast<Enum100>(i);
        REQUIRE(static_cast<Enum100>(f) != prev_enum);
    }
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