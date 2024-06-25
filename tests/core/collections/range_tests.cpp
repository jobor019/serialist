#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/collections/range.h"
#include <iostream>

TEST_CASE("Continuous Range") {
    ContinuousRange<double> range(0, 1); // TODO
}


template<typename T>
inline void eval_integral_range(const DiscreteRange<T>& range, T start, T end, std::size_t steps, bool include_end) {
    REQUIRE(range.size() == steps);

    if (include_end) {
        REQUIRE(range.at(range.size() - 1) == end);
    } else {
        REQUIRE(range.at(range.size() - 1) == (end - 1));
    }

    for (std::size_t i = 0; i < steps; ++i) {
        REQUIRE(range.at(i) == start + static_cast<T>(i));
    }
}

TEST_CASE("Integral DiscreteRange defined by size") {
    std::size_t size = 10000;

    for (auto include_end: Vec<bool>{false, true}) {

        SECTION("Integer range from 0") {
            int start = 0;
            int end = start + static_cast<int>(size);
            auto range = DiscreteRange<int>::from_size(start, end, size, include_end);
            eval_integral_range(range, start, end, size, include_end);
        }

        SECTION("Integer range to max") {
            int start = std::numeric_limits<int>::max() - static_cast<int>(size);
            int end = start + static_cast<int>(size);
            auto range = DiscreteRange<int>::from_size(start, end, size, include_end);
            eval_integral_range(range, start, end, size, include_end);
        }

        SECTION("Long range from 0") {
            long start = 0;
            long end = start + static_cast<long>(size);
            auto range = DiscreteRange<long>::from_size(start, end, size, include_end);
            eval_integral_range(range, start, end, size, include_end);
        }

        SECTION("Long range towards double::digits") {
            // Note: Only tested up to size of the mantissa of double, will yield incorrect results above this value
            for (std::size_t i = 0; i < std::numeric_limits<double>::digits + 1; ++i) {
                auto end = static_cast<long>(std::pow(2, i));
                auto start = end - static_cast<long>(size);
                auto range = DiscreteRange<long>::from_size(start, end, size, include_end);
                eval_integral_range(range, start, end, size, include_end);
            }
        }

        SECTION("Long range negative values") {
            for (std::size_t i = 0; i < std::numeric_limits<double>::digits + 1; ++i) {
                auto start = -static_cast<long>(std::pow(2, i));
                auto end = start + static_cast<long>(size);
                auto range = DiscreteRange<long>::from_size(start, end, size, include_end);
                eval_integral_range(range, start, end, size, include_end);
            }
        }
    }
}


TEST_CASE("Integral DiscreteRange defined by (integral) step_size") {
    auto num_steps = 10000;

    for (auto include_end: Vec<bool>{false, true}) {
        std::cout << "evaluating include_end = " << include_end << std::endl;

        for (auto step_size: Vec<long>{1, 2, 10, 100, 1000, 10000}) {
            std::cout << "evaluating step_size = " << step_size << std::endl;
            // Note: Only tested up to size of the mantissa of double, will yield incorrect results above this value
            for (std::size_t i = 1; i < std::numeric_limits<double>::digits; ++i) {

                auto start = static_cast<long>(std::pow(2, i));
                auto end = start + step_size * num_steps;

                auto range = DiscreteRange<long>::from_step(start, end, step_size, include_end);

                REQUIRE(range.at(range.size() - 1) <= end);

                auto v = range.at(0);
                REQUIRE(v == start);

                for (std::size_t j = 1; j < range.size(); ++j) {
                    REQUIRE(range.at(j) == v + step_size);
                    v = range.at(j);
                }
            }
        }
    }
}

TEST_CASE("Integral DiscreteRange defined by floating step_size") {

}

TEST_CASE("Invalid integral ranges") {}

inline void eval_floating_range(const DiscreteRange<double>& range, double start, double end
                                , std::size_t steps, bool include_end) {

    double expected_step_size = (end - start) / static_cast<double>(steps - static_cast<std::size_t>(include_end));

    REQUIRE_THAT(range.at(range.size() - 1), Catch::Matchers::WithinAbs(end - expected_step_size, 1e-8));
    REQUIRE_THAT(range.at(0), Catch::Matchers::WithinAbs(0, 1e-8));

    for (std::size_t i = 0; i < range.size(); ++i) {
        REQUIRE_THAT(range.at(i)
                     , Catch::Matchers::WithinAbs(start + static_cast<double>(i) * expected_step_size, 1e-8));
    }

    REQUIRE(range.size() == steps);
}

TEST_CASE("Floating unit range DiscreteRange defined by num_steps") {
    std::size_t size = 10000;

    for (auto include_end: Vec<bool>{false, true}) {

        SECTION("Unit Range") {
            double start = 0.0;
            double end = 1.0;
            auto range = DiscreteRange<double>::from_size(start, end, size, include_end);
            eval_floating_range(range, start, end, size, include_end);
        }
    }

    // TODO: More tests
}

TEST_CASE("Floating unit range DiscreteRange defined by step_size") {
    // TODO
}

TEST_CASE("Floating DiscreteRange defined by num_steps") {
    // TODO
}

TEST_CASE("Floating DiscreteRange defined by step_size") {
    // TODO
}

TEST_CASE("Invalid floating ranges") {
    // TODO
}


TEST_CASE("Map") {
    SECTION("Integral range") {
        std::size_t size = 100;

        auto range = DiscreteRange<std::size_t>::from_size(0, size, size, false);
        for (std::size_t i = 0; i < size; ++i) {
            double unit_index = static_cast<double>(i) / static_cast<double>(size - 1);
            REQUIRE(range.map(unit_index) == i);
        }
    }

    // TODO: More tests required
}

TEST_CASE("Inverse") {
    SECTION("Integral range") {
        std::size_t size = 100;

        auto range = DiscreteRange<std::size_t>::from_size(0, size, size, false);

        for (std::size_t i = 0; i < size; ++i) {
            double expected_unit_index = static_cast<double>(i) / static_cast<double>(size - 1);
            REQUIRE(range.inverse(i) == expected_unit_index);
        }
    }
}
