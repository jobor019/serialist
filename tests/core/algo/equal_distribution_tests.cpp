
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/algo/temporal/equal_duration_sampling.h"
#include "core/algo/histogram.h"

TEST_CASE("Equal Duration Sampling Tests") {
    auto lb = 0.25;
    auto ub = 4.0;
    EqualDurationSampling sampler(lb, ub, 0);

    SECTION("generated_numbers_within_specified_bounds") {
        for (int i = 0; i < 10000; ++i) {
            double value = sampler.next();
            REQUIRE(value >= lb);
            REQUIRE(value < ub);
        }
    }

    SECTION("generated_numbers_follow_distribution_chi_square_test") {
        const int num_bins = 100;
        const int num_samples = 10000;

        auto observed = Vec<double>::allocated(num_samples);
        for (int i = 0; i < num_samples; ++i) {
            observed.append(sampler.next());
        }

        auto observed_hist = Histogram<double>(observed, lb, ub, num_bins);

        auto bins = observed_hist.get_bins();

        auto step = bins[1] - bins[0];

        auto expected = bins.cloned()
                .add(step / 2)
                .map([&sampler](const auto& x) { return sampler.pdf(x); });

        auto p_observed = observed_hist.get_counts().as_type<double>().normalize_max();

        auto chi_square = ((p_observed - expected).pow(2) / expected).sum();

        REQUIRE(chi_square < 1.0); // 1.0 completely arbitrarily chosen, could be any value
    }
}


