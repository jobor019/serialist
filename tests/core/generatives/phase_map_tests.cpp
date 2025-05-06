
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/policies/policies.h"
#include "node_runner.h"
#include "core/generatives/phase_map.h"

#include "generators.h"
#include "matchers/m11.h"
#include "matchers/m1m.h"
#include "matchers/m1s.h"

using namespace serialist;
using namespace serialist::test;


TEST_CASE("PhaseMap: Phase is mapped to durations", "[phase_map]") {
    PhaseMapWrapper w;

    auto& pm = w.phase_map_node;
    auto& cursor = w.cursor;
    auto& durations = w.durations;

    NodeRunner runner(&pm);


    SECTION("Single / identify duration") {
        durations.set_values(1.0);

        auto cursor_position = GENERATE(0.0, 0.5, 0.99);
        CAPTURE(cursor_position);

        cursor.set_values(cursor_position);
        auto r = runner.step();
        REQUIRE_THAT(r, m11::eqf(cursor_position));
    }


    SECTION("Multiple durations") {
        durations.set_values(Voices<double>::singular({0.25, 0.5, 0.25}));

        auto [cursor_position, expected_output] = GENERATE(
            table<double, double>({
                // First segment (duration=0.25, segment: 0.0 to 0.25)
                {0.0, 0.0},
                {0.125, 0.5},
                {0.25 - EPSILON, Phase::max()},

                // Second segment (duration=0.5, segment: 0.25 to 0.75)
                {0.25 + EPSILON, 0.0},
                {0.35, 0.2},
                {0.45, 0.4},
                {0.55, 0.6},
                {0.65, 0.8},
                {0.75 - EPSILON, Phase::max()},

                // Third segment (duration=0.25, segment: 0.75 to Phase::max())
                {0.75 + EPSILON, 0.0},
                {Phase::max(), Phase::max()},

                // Wrap-around
                {1.0 + EPSILON, 0.0},
            }));

        CAPTURE(cursor_position, expected_output);

        cursor.set_values(cursor_position);
        auto r = runner.step();
        REQUIRE_THAT(r, m11::eqf(expected_output));
    }

    SECTION("Negative duration equals pause") {
        durations.set_values(Voices<double>::singular({0.25, -0.5, 0.25}));

        // First segment
        cursor.set_values(0.0);
        REQUIRE_THAT(runner.step(), m11::eqf(0.0));
        cursor.set_values(0.25 - EPSILON);
        REQUIRE_THAT(runner.step(), m11::eqf(Phase::max()));

        // Second segment (pause)
        cursor.set_values(0.25 + EPSILON);
        REQUIRE_THAT(runner.step(), m11::emptyf());
        cursor.set_values(0.5);
        REQUIRE_THAT(runner.step(), m11::emptyf());
        cursor.set_values(0.75 - EPSILON);
        REQUIRE_THAT(runner.step(), m11::emptyf());

        // Third segment
        cursor.set_values(0.75 + EPSILON);
        REQUIRE_THAT(runner.step(), m11::eqf(0.0));
        cursor.set_values(Phase::max());
        REQUIRE_THAT(runner.step(), m11::eqf(Phase::max()));
    }
}


TEST_CASE("PhaseMap: Single duration maps to corresponding tuplet") {
    PhaseMapWrapper w;

    auto& pm = w.phase_map_node;
    auto& cursor = w.cursor;
    auto& durations = w.durations;

    durations.set_values(5.0); // quintuplets

    NodeRunner runner(&pm);
    auto [cursor_position, expected_output] = GENERATE(
            table<double, double>({
                {0.0, 0.0},
                {0.1, 0.5},
                {0.2 - EPSILON, Phase::max()},

                {0.2 + EPSILON, 0.0},
                {0.3, 0.5},
                {0.4 - EPSILON, Phase::max()}
            }));

    CAPTURE(cursor_position, expected_output);

    cursor.set_values(cursor_position);
    REQUIRE_THAT(runner.step(), m11::eqf(expected_output));

    cursor.set_values(1.0/5.0 - EPSILON);

}


TEST_CASE("PhaseMap: Each voice can have a different mapping", "[phase_map]") {
    PhaseMapWrapper w;

    auto& pm = w.phase_map_node;
    auto& cursor = w.cursor;
    auto& durations = w.durations;

    NodeRunner runner(&pm);

    durations.set_values({
        Voice<double>{1.0},
        Voice<double>{0.5, 0.5},
        Voice<double>{0.25, 0.25, 0.25, 0.25}
    });

    cursor.set_values(0.0);
    REQUIRE_THAT(runner.step(), m1s::eqf(Voice<double>{0.0, 0.0, 0.0}));

    cursor.set_values(0.25 + EPSILON);
    REQUIRE_THAT(runner.step(), m1s::eqf(Voice<double>{0.25, 0.5, 0.0}));

    cursor.set_values(0.5 + EPSILON);
    REQUIRE_THAT(runner.step(), m1s::eqf(Voice<double>{0.5, 0.0, 0.0}));

    cursor.set_values(Phase::max());
    REQUIRE_THAT(runner.step(), m1s::eqf(Voice<double>{Phase::max(), Phase::max(), Phase::max()}));


}