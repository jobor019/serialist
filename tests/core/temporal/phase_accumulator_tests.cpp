
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/policies/policies.h"
#include "core/temporal/phase_accumulator.h"

#include "node_runner.h"
#include "generators.h"

using namespace serialist;
using namespace serialist::test;


static PhaseAccumulator initialize_phase_accumulator(double step_size, double phase, PaMode mode) {
    PhaseAccumulator p;
    p.set_mode(mode);
    p.set_step_size(step_size);
    p.set_offset(DomainDuration{phase, DomainType::ticks});
    return p;
}


TEST_CASE("m_phasor stepped", "[phasor]") {
    SECTION("unit m_phasor") {
        double step = 0.1;
        auto p = initialize_phase_accumulator(step, 0.0, PaMode::triggered);

        double x;
        TimePoint t; // value irrelevant when stepped

        for (int i = 0; i < 100; ++i) {
            x = p.process(t, true);
            REQUIRE_THAT(x, Catch::Matchers::WithinAbs(std::fmod(step * i, 1.0), 1e-8));
            REQUIRE(x < 1.0);
        }
    }

    SECTION("Negative step size") {
        double step = -0.1;
        auto p = initialize_phase_accumulator(step, 0.0, PaMode::triggered);

        double x;
        double y = 0;
        TimePoint t; // value irrelevant when stepped
        for (int i = 0; i < 100; ++i) {
            x = p.process(t, true);
            if (y < 0) {
                y += 1.0;
            }
            REQUIRE_THAT(x, Catch::Matchers::WithinAbs(y, 1e-8));
            REQUIRE(x < 1.0);
            REQUIRE(x >= 0);
            y += step;
        }
    }

    SECTION("Variable step size") {
        auto step = 0.0;
        auto p = initialize_phase_accumulator(step, 0.0, PaMode::triggered);
        TimePoint t; // value irrelevant when stepped

        REQUIRE_THAT(p.process(t, true), Catch::Matchers::WithinAbs(0.0, 1e-8));
        p.set_step_size(0.2);
        REQUIRE_THAT(p.process(t, true), Catch::Matchers::WithinAbs(0.2, 1e-8));
        p.set_step_size(0.8);
        REQUIRE_THAT(p.process(t, true), Catch::Matchers::WithinAbs(0.0, 1e-8));
    }
}