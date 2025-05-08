#include "core/policies/policies.h"
#include "node_runner.h"
#include "generatives/random_node.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>


#include "generators.h"
#include "matchers/m1m.h"
#include "matchers/m11.h"
#include "matchers/m1s.h"
#include "matchers/mms.h"

using namespace serialist;
using namespace serialist::test;

using AvoidRepetitions = RandomHandler::AvoidRepetitions;
using Mode = RandomHandler::Mode;

TEST_CASE("Random(Node): ctor", "[random_node]") {
    RandomWrapper<> w;

    NodeRunner runner{&w.random};

    auto r = runner.step();
    REQUIRE_THAT(r, !m11::emptyf());
}


TEST_CASE("Random(Node): quantized values handle index round-trip correctly", "[random_node]") {
    RandomWrapper<> w;
    w.random.set_seed(0);
    auto& q = w.num_quantization_steps;
    NodeRunner runner{&w.random};

    auto [size, step_size, range] = GENERATE(
        table<std::size_t, double, double>({
            {2, 0.5, 0.5},
            {4, 0.25, 0.75},
            {10, 0.1, 0.9},
            })
    );
    q.set_values(size);

    auto r = runner.step_n(1000);

    // range is capped to [0.0, range + epsilon), not [0.0, 1.0]
    REQUIRE_THAT(r, m11::in_rangef(0.0, range + EPSILON, MatchType::all));

    // All steps from [0.0 to range] are represented at least once
    for (double step = 0.0; step < range; step += step_size) {
        REQUIRE_THAT(r, m11::eqf(step, MatchType::any));
    }
}


TEST_CASE("Random(Node): Repetitions mode does not change value outcomes", "[random_node]") {
    // Note: this test case was added due to a bug discovered at runtime
    RandomWrapper<> w;
    w.random.set_seed(0);
    NodeRunner runner{&w.random};

    auto& repetitions = w.repetition_strategy;
    auto& quantization = w.num_quantization_steps;


    auto rmode = GENERATE(AvoidRepetitions::off, AvoidRepetitions::chordal, AvoidRepetitions::sequential);
    CAPTURE(rmode);

    quantization.set_values(2);
    repetitions.set_value(rmode);

    // We only expect to see 0.0 or 0.5 as output
    auto r = runner.step_n(100);
    REQUIRE_THAT(r, m11::in_rangef(0.0, 0.5 + EPSILON, MatchType::all));
    REQUIRE_THAT(r, m11::eqf(0.0, MatchType::any));
    REQUIRE_THAT(r, m11::eqf(0.5, MatchType::any));
}


TEST_CASE("Random(Node): Weighted will not select element with zero weight (with single exception)", "[random_node]") {
    // Note: this test case was added due to a bug discovered at runtime
    RandomWrapper<> w;
    w.random.set_seed(0);
    NodeRunner runner{&w.random};

    w.mode.set_value(Mode::weighted);

    auto rmode = GENERATE(AvoidRepetitions::off, AvoidRepetitions::chordal, AvoidRepetitions::sequential);
    CAPTURE(rmode);

    auto& repetitions = w.repetition_strategy;
    repetitions.set_value(rmode);

    SECTION("Normal case") {
        // Valid outcomes: 0.0 (index 0, 50%), 0.6 (index 3, 25%), 0.8 (index 4, 25%)
        w.weights.set_values(Voices<double>::transposed({0.5, 0.0, 0.0, 0.25, 0.25}));

        auto r = runner.step_n(100);
        REQUIRE_THAT(r, m11::in_setf(Voice<double>{0.0, 0.6, 0.8}, MatchType::all));
        REQUIRE_THAT(r, m11::eqf(0.0, MatchType::any));
        REQUIRE_THAT(r, m11::eqf(0.6, MatchType::any));
        REQUIRE_THAT(r, m11::eqf(0.8, MatchType::any));
    }

    SECTION("Exception: When all weights are zero, output is 0.0") {
        w.weights.set_values(Voices<double>::transposed({0.0, 0.0, 0.0, 0.0, 0.0}));

        auto r = runner.step_n(100);
        REQUIRE_THAT(r, m11::eqf(0.0, MatchType::all));
    }

    SECTION("Edge case: if first weight is zero and a non-zero weight exists, output is never 0.0") {
        w.weights.set_values(Voices<double>::transposed({0.0, 0.0, 0.0, 0.0, 1.0}));

        auto r = runner.step_n(100);
        REQUIRE_THAT(r, m11::eqf(0.8, MatchType::all));
    }
}