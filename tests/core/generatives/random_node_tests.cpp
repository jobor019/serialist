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


TEST_CASE("Random(Node): AvoidRepetitions::chordal repetitions are handled correctly", "[random_node]") {
    RandomWrapper<> w;
    w.random.set_seed(0);
    NodeRunner runner{&w.random};

    w.repetition_strategy.set_value(AvoidRepetitions::chordal);

    auto& size = w.chord_size;
    auto& weights = w.weights;
    auto& mode = w.mode;
    auto& quantization = w.num_quantization_steps;

    SECTION("Mode::uniform does not contain duplicates") {
        // uniform possible values: [0, 0.25, 0.5, 0.75]
        mode.set_value(Mode::uniform);

        auto size_value = GENERATE(1, 2, 3, 4);
        CAPTURE(size_value);
        size.set_values(size_value);

        auto r = runner.step_n(1000);
        REQUIRE_THAT(r, m1m::sizef(size_value, MatchType::all));

    }

    SECTION("Mode::uniform where chord size is greater than quantization steps") {
        mode.set_value(Mode::uniform);
        size.set_values(5);
        quantization.set_values(4);

        auto r = runner.step_n(1000);
        REQUIRE_THAT(r, m1m::sizef(5, MatchType::all));
        REQUIRE_THAT(r, m1m::containsf_duplicates(MatchType::all));
    }

    SECTION("Mode::weighted where chord size is smaller than or eq number of weights does not contain duplicates") {
        mode.set_value(Mode::weighted);

        // 5 weights. actual weights are irrelevant, just need to be non-zero
        weights.set_values(Voices<double>::transposed({0.1, 0.3, 1, 0.3, 0.1}));

        auto size_value = GENERATE(1, 2, 3, 4, 5);
        CAPTURE(size_value);
        size.set_values(size_value);

        auto r = runner.step_n(1000);
        REQUIRE_THAT(r, m1m::sizef(size_value, MatchType::all));
        REQUIRE_THAT(r, !m1m::containsf_duplicates(MatchType::any));

    }

    SECTION("Mode::weighted where chord size larger than number of weights has correct size with duplicates") {
        mode.set_value(Mode::weighted);

        // 4 weights. actual weights are irrelevant, just need to be non-zero
        weights.set_values(Voices<double>::transposed({0.9, 0.9, 0.1, 0.9}));

        auto size_value = GENERATE(5, 6, 10);
        CAPTURE(size_value);
        size.set_values(size_value);

        auto r = runner.step_n(1000);
        REQUIRE_THAT(r, m1m::sizef(size_value, MatchType::all));
        REQUIRE_THAT(r, m1m::containsf_duplicates(MatchType::all));
    }

    SECTION("Mode::weighted where chord size is same as number of weights but some weights are 0") {
        mode.set_value(Mode::weighted);

        // 4 weights with 2 being 0.0
        weights.set_values(Voices<double>::transposed({0.9, 0.0, 0.0, 0.9}));

        size.set_values(4);

        auto r = runner.step_n(1000);
        REQUIRE_THAT(r, m1m::sizef(4, MatchType::all));
        REQUIRE_THAT(r, m1m::containsf_duplicates(MatchType::all));
    }
}
