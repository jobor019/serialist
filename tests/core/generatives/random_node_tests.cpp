
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