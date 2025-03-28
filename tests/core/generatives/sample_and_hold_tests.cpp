
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/policies/policies.h"
#include "node_runner.h"
#include "generators.h"
#include "generatives/sample_and_hold.h"


using namespace serialist;
using namespace serialist::test;


TEST_CASE("SampleAndHold: wrapper ctor", "[sample_and_hold]") {
    SampleAndHoldWrapper<> w;
}