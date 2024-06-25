
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/collections/vec.h"
#include "core/algo/histogram.h"


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

    SECTION("Non-sorted input") {
        Vec values = {4, 4, 4, 4, 3, 3, 3, 2, 2, 1};
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


    SECTION("With specified, discrete bins") {
        Vec values = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4};
        Vec bins = {1, 2, 3, 4, 5};
        auto histogram2 = Histogram<int>::with_discrete_bins(values, bins);
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


TEST_CASE("Float-point Histogram construction and value/count retrieval") {
    SECTION("Empty input") {
        Vec<double> empty_values;
        Histogram<double> histogram(empty_values, 0.0, 1.0, 10);

        REQUIRE(histogram.get_bins().size() == 10);
        histogram.get_counts().print();
        REQUIRE(histogram.get_counts().size() == 10);
        REQUIRE_THAT(histogram.get_counts().sum(), Catch::Matchers::WithinAbs(0, 1e-6));
    }

    SECTION("Non-empty input") {
        Vec values = {1.2, 2.3, 2.3, 3.4, 3.4, 3.4, 4.5, 4.5, 4.5, 4.5};
        Histogram histogram(values, 1.0, 5.0, 4);

        const Vec<double>& histogram_values = histogram.get_bins();
        const Vec<std::size_t>& histogram_counts = histogram.get_counts();

        REQUIRE(histogram_values.size() == 4);
        REQUIRE(histogram_counts.size() == 4);

        REQUIRE_THAT(histogram_values[0], Catch::Matchers::WithinAbs(1.0, 1e-6));
        REQUIRE_THAT(histogram_values[1], Catch::Matchers::WithinAbs(2.0, 1e-6));
        REQUIRE_THAT(histogram_values[2], Catch::Matchers::WithinAbs(3.0, 1e-6));
        REQUIRE_THAT(histogram_values[3], Catch::Matchers::WithinAbs(4.0, 1e-6));

        REQUIRE(histogram_counts[0] == 1);
        REQUIRE(histogram_counts[1] == 2);
        REQUIRE(histogram_counts[2] == 3);
        REQUIRE(histogram_counts[3] == 4);
    }

    SECTION("Non-sorted input") {
        Vec values = {4.5, 4.5, 4.5, 4.5, 3.4, 3.4, 3.4, 2.3, 2.3, 1.2};
        Histogram histogram(values, 1.0, 5.0, 4);

        const Vec<double>& histogram_values = histogram.get_bins();
        const Vec<std::size_t>& histogram_counts = histogram.get_counts();

        REQUIRE(histogram_values.size() == 4);
        REQUIRE(histogram_counts.size() == 4);

        REQUIRE_THAT(histogram_values[0], Catch::Matchers::WithinAbs(1.0, 1e-6));
        REQUIRE_THAT(histogram_values[1], Catch::Matchers::WithinAbs(2.0, 1e-6));
        REQUIRE_THAT(histogram_values[2], Catch::Matchers::WithinAbs(3.0, 1e-6));
        REQUIRE_THAT(histogram_values[3], Catch::Matchers::WithinAbs(4.0, 1e-6));

        REQUIRE(histogram_counts[0] == 1);
        REQUIRE(histogram_counts[1] == 2);
        REQUIRE(histogram_counts[2] == 3);
        REQUIRE(histogram_counts[3] == 4);
    }

//    SECTION("With specified bins") {
//        auto values = {1.2, 2.3, 2.3, 3.4, 3.4, 3.4, 4.5, 4.5, 4.5, 4.5};
//        auto bins = {1.0, 2.0, 3.0, 4.0, 5.0};
//        Histogram<double> histogram2(values, bins);
//        const Vec<double>& histogram2_values = histogram2.get_bins();
//        const Vec<std::size_t>& histogram2_counts = histogram2.get_counts();
//
//        REQUIRE(histogram2_values.size() == 5);
//        REQUIRE(histogram2_counts.size() == 5);
//
//        REQUIRE_THAT(histogram2_values[0], Catch::Matchers::WithinAbs(1.0, 1e-6));
//        REQUIRE_THAT(histogram2_values[1], Catch::Matchers::WithinAbs(2.0, 1e-6));
//        REQUIRE_THAT(histogram2_values[2], Catch::Matchers::WithinAbs(3.0, 1e-6));
//        REQUIRE_THAT(histogram2_values[3], Catch::Matchers::WithinAbs(4.0, 1e-6));
//        REQUIRE_THAT(histogram2_values[4], Catch::Matchers::WithinAbs(5.0, 1e-6));
//
//        REQUIRE(histogram2_counts[0] == 1);
//        REQUIRE(histogram2_counts[1] == 2);
//        REQUIRE(histogram2_counts[2] == 3);
//        REQUIRE(histogram2_counts[3] == 4);
//        REQUIRE(histogram2_counts[4] == 0);
//    }
}



