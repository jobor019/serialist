
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/algo/collections/vec.h"
#include "core/algo/stat.h"


TEST_CASE("Histogram construction and value/count retrieval") {
    SECTION("Empty input") {
        Vec<int> empty_values;
        Histogram histogram(empty_values);

        REQUIRE(histogram.get_bins().empty());
        REQUIRE(histogram.get_counts().empty());
    }

    SECTION("Non-empty input") {
        Vec values = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4};
        Histogram histogram(values);

        const Vec<int>& histogram_values = histogram.get_bins();
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

    SECTION("With specified bins") {
        Vec<int> values = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4};
        Vec<int> bins = {1, 2, 3, 4, 5};
        Histogram<int> histogram2(values, bins);
        const Vec<int>& histogram2_values = histogram2.get_bins();
        const Vec<std::size_t>& histogram2_counts = histogram2.get_counts();

        REQUIRE(histogram2_values.size() == 5);
        REQUIRE(histogram2_counts.size() == 5);

        REQUIRE(histogram2_values[0] == 1);
        REQUIRE(histogram2_values[1] == 2);
        REQUIRE(histogram2_values[2] == 3);
        REQUIRE(histogram2_values[3] == 4);
        REQUIRE(histogram2_values[4] == 5);

        REQUIRE(histogram2_counts[0] == 1);
        REQUIRE(histogram2_counts[1] == 2);
        REQUIRE(histogram2_counts[2] == 3);
        REQUIRE(histogram2_counts[3] == 4);
        REQUIRE(histogram2_counts[4] == 0);

    }
}


