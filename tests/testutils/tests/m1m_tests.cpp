#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "matchers/matchers_common.h"
#include "matchers/m1m.h"

using namespace serialist::test;

TEST_CASE("testutils::m1m facet comparisons", "[testutils][m1m]") {
    REQUIRE_THAT(RunResult<Facet>::dummy(Voice<double>{0.4, 0.4}.as_type<Facet>(), false), m1m::containsf_duplicates());
    REQUIRE_THAT(RunResult<Facet>::dummy(Voice<double>{0.4, 0.39}.as_type<Facet>(), false), !m1m::containsf_duplicates());
}


TEST_CASE("testutils::m1m trigger comparisons", "[testutils][m1m]") {
    SECTION("sortedt: valid cases") {
        // Single pulse_on is sorted
        REQUIRE_THAT(RunResult<Trigger>::dummy(Vec{
                         Trigger::with_manual_id(Trigger::Type::pulse_on, 3)
                         }, false), m1m::sortedt());

        // Single pulse_off is sorted
        REQUIRE_THAT(RunResult<Trigger>::dummy(Vec{
                         Trigger::with_manual_id(Trigger::Type::pulse_off, 2)
                         }, false), m1m::sortedt());

        // pulse_off followed by pulse_on with higher ID is sorted
        REQUIRE_THAT(RunResult<Trigger>::dummy(Vec{
                         Trigger::with_manual_id(Trigger::Type::pulse_off, 1)
                         , Trigger::with_manual_id(Trigger::Type::pulse_on, 2)
                         }, false), m1m::sortedt());

        // Multiple pulse_on in ascending ID order is sorted
        REQUIRE_THAT(RunResult<Trigger>::dummy(Vec{
                         Trigger::with_manual_id(Trigger::Type::pulse_on, 3)
                         , Trigger::with_manual_id(Trigger::Type::pulse_on, 4)
                         , Trigger::with_manual_id(Trigger::Type::pulse_on, 5)
                         }, false), m1m::sortedt());

        // Multiple pulse_on/pulse_off pairs in ascending ID order is sorted
        REQUIRE_THAT(RunResult<Trigger>::dummy(Vec{
                         Trigger::with_manual_id(Trigger::Type::pulse_on, 1)
                         , Trigger::with_manual_id(Trigger::Type::pulse_off, 1)
                         , Trigger::with_manual_id(Trigger::Type::pulse_on, 2)
                         , Trigger::with_manual_id(Trigger::Type::pulse_off, 2)
                         }, false), m1m::sortedt());
    }

    SECTION("sortedt: invalid cases") {
        // Higher ID before lower ID is not sorted
        REQUIRE_THAT(RunResult<Trigger>::dummy(Vec{
                         Trigger::with_manual_id(Trigger::Type::pulse_on, 3)
                         , Trigger::with_manual_id(Trigger::Type::pulse_off, 2)
                         }, false), !m1m::sortedt());

        // pulse_off before corresponding pulse_on is not sorted
        REQUIRE_THAT(RunResult<Trigger>::dummy(Vec{
                         Trigger::with_manual_id(Trigger::Type::pulse_off, 3)
                         , Trigger::with_manual_id(Trigger::Type::pulse_on, 3)
                         }, false), !m1m::sortedt());

        // Both conditions violated: higher ID before lower ID and pulse_off before pulse_on
        REQUIRE_THAT(RunResult<Trigger>::dummy(Vec{
                         Trigger::with_manual_id(Trigger::Type::pulse_off, 5)
                         , Trigger::with_manual_id(Trigger::Type::pulse_on, 3)
                         }, false), !m1m::sortedt());

        // Mixed valid and invalid: starts valid but then has out-of-order IDs
        REQUIRE_THAT(RunResult<Trigger>::dummy(Vec{
                         Trigger::with_manual_id(Trigger::Type::pulse_on, 1)
                         , Trigger::with_manual_id(Trigger::Type::pulse_off, 1)
                         , Trigger::with_manual_id(Trigger::Type::pulse_on, 5)
                         , Trigger::with_manual_id(Trigger::Type::pulse_on, 3)
                         }, false), !m1m::sortedt());
    }
}


TEST_CASE("testutils::m1m event comparisons", "[testutils][m1m]") {
    auto chord = RunResult<Event>::dummy({Event::note(60, 100, 1), Event::note(64, 100, 2)}, false);
    auto chord2 = RunResult<Event>::dummy(
        {Event::note(60, 100, 1)
         , Event::note(64, 100, 2)
         , Event::note(67, 100, 1)
        }
        , false);
    assert(chord.voices()->size() == 1); // in case implementation changes
    assert((*chord.voices())[0].size() == 2);

    REQUIRE_THAT(chord, m1m::contains_note({60, 100, 1}));
    REQUIRE_THAT(chord, !m1m::contains_note({61, 100, 1}));

    REQUIRE_THAT(chord, m1m::equals_chord({
                     {60, 100, 1},
                     {64}
                     }));
    // wrong channel on second chord
    REQUIRE_THAT(chord, !m1m::equals_chord({{60, 100, 1}, {64, 100, 15}}));

    REQUIRE_THAT(chord, m1m::contains_chord({{60}, {64}}));
    REQUIRE_THAT(chord, !m1m::contains_chord({{61}, {64}}));
}
