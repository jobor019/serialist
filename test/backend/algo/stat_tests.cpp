
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/algo/collections/vec.h"
#include "core/algo/stat.h"


TEST_CASE("Histogram construction and value/count retrieval") {
    SECTION("Empty input") {
        Vec<int> empty_values;
        Histogram histogram(empty_values);

        REQUIRE(histogram.get_values().empty());
        REQUIRE(histogram.get_counts().empty());
    }

    SECTION("Non-empty input") {
        Vec values = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4};
        Histogram histogram(values);

        const Vec<int>& histogram_values = histogram.get_values();
        const Vec<std::size_t>& histogram_counts = histogram.get_counts();

        REQUIRE(histogram_values.size() == 4);
        REQUIRE(histogram_counts.size() == 4);

        REQUIRE(histogram_values[0] == 1);
        REQUIRE(histogram_values[1] == 2);
        REQUIRE(histogram_values[2] == 3);
        REQUIRE(histogram_values[3] == 4);

        REQUIRE(histogram_counts[0] == 1);
        REQUIRE(histogram_counts[1] == 2);
        REQUIRE(histogram_counts[2] == 3);
        REQUIRE(histogram_counts[3] == 4);
    }
}

