
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "serialist/core/policies/policies.h"
#include "node_runner.h"
#include "core/generatives/scaler.h"
#include "core/types/time_point.h"

#include "matchers/m11.h"
#include "matchers/m1m.h"
#include "matchers/m1s.h"

using namespace serialist;

TEST_CASE("Scaler: ctor", "[scaler]") {
    auto scaler = ScalerWrapper();
}


TEST_CASE("Scaler: basic operation (temp)", "[scaler]") {

    auto w = ScalerWrapper();
    NodeRunner runner{&w.scaler_node};

    SECTION("Scale single value to output range") {
        w.output_low.set_values(10);
        w.output_high.set_values(20);

        w.value.set_values(0.5);
        REQUIRE_THAT(runner.step(), m11::eqf(15));

        w.value.set_values(0.0);
        REQUIRE_THAT(runner.step(), m11::eqf(10));

        w.value.set_values(1.0);
        REQUIRE_THAT(runner.step(), m11::eqf(20));
    }

    SECTION("Scale multiple values to output range") {
        w.output_low.set_values(10);
        w.output_high.set_values(20);

        w.value.set_values(Voices<double>::transposed({0.0, 0.5, 1.0}));
        REQUIRE_THAT(runner.step(), m1s::eqf(Voice<double>{10, 15, 20}));
    }
}