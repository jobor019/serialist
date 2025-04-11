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
