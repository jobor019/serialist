
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/policies/policies.h"
#include "generatives/index_node.h"

#include "node_runner.h"
#include "generators.h"

#include "matchers/m11.h"

using namespace serialist;
using namespace serialist::test;


TEST_CASE("IndexNode: wrapper ctor", "[index_node]") {
    IndexWrapper<> w;
}

TEST_CASE("IndexNode: basic operation", "[index_node]") {
    IndexWrapper<> w;

    auto& trigger = w.trigger;
    auto& num_steps = w.num_steps;
    auto& stride = w.stride;
    auto& reset_trigger = w.reset;

    NodeRunner runner{&w.index_node};

    SECTION("Unit Stride") {
        num_steps.set_values(5);
        stride.set_values(1.0);
        trigger.set_values(Trigger::pulse_on());
        REQUIRE_THAT(runner.step(), m11::eqf(0));
        REQUIRE_THAT(runner.step(), m11::eqf(1));
        REQUIRE_THAT(runner.step(), m11::eqf(2));
        REQUIRE_THAT(runner.step(), m11::eqf(3));
        REQUIRE_THAT(runner.step(), m11::eqf(4));
        REQUIRE_THAT(runner.step(), m11::eqf(0));
    }

    SECTION("Negative stride") {
        num_steps.set_values(5);
        stride.set_values(-1.0);
        trigger.set_values(Trigger::pulse_on());
        REQUIRE_THAT(runner.step(), m11::eqf(4));
        REQUIRE_THAT(runner.step(), m11::eqf(3));
        REQUIRE_THAT(runner.step(), m11::eqf(2));
        REQUIRE_THAT(runner.step(), m11::eqf(1));
        REQUIRE_THAT(runner.step(), m11::eqf(0));
        REQUIRE_THAT(runner.step(), m11::eqf(4));
    }

    SECTION("Non-integral stride") {
        num_steps.set_values(5);
        stride.set_values(1.5);
        trigger.set_values(Trigger::pulse_on());
        REQUIRE_THAT(runner.step(), m11::eqf(0));
        REQUIRE_THAT(runner.step(), m11::eqf(1)); // 1.5
        REQUIRE_THAT(runner.step(), m11::eqf(3)); // 3.0
        REQUIRE_THAT(runner.step(), m11::eqf(4)); // 4.5
        REQUIRE_THAT(runner.step(), m11::eqf(1)); // 6.0 % 5 == 1
    }

    SECTION("Reset trigger resets to 0 if stride is non-negative") {
        auto stride_v = GENERATE(0.0, 1.0, 2.0, 3.0);
        num_steps.set_values(5);
        stride.set_values(stride_v);
        trigger.set_values(Trigger::pulse_on());

        REQUIRE_THAT(runner.step(), m11::eqf(0));
        REQUIRE_THAT(runner.step(), m11::eqf(stride_v));

        reset_trigger.set_values(Trigger::pulse_on());
        REQUIRE_THAT(runner.step(), m11::eqf(0));
    }

    SECTION("Reset trigger resets to num_steps if stride is negative") {
        auto stride_v = GENERATE(-1.0, -2.0, -3.0);
        num_steps.set_values(5);
        stride.set_values(stride_v);
        trigger.set_values(Trigger::pulse_on());

        REQUIRE_THAT(runner.step(), m11::eqf(4));
        REQUIRE_THAT(runner.step(), m11::eqf(4 + stride_v));

        reset_trigger.set_values(Trigger::pulse_on());
        REQUIRE_THAT(runner.step(), m11::eqf(4));
    }
}