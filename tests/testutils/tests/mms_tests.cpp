
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "matchers/matchers_common.h"
#include "matchers/mms.h"

using namespace serialist::test;

using NC = NoteComparator;

TEST_CASE("testutils::mms midi note event comparisons", "[testutils][mms]") {
    auto c = Voices<Event>{
        {Event::note(60, 100, 1)},
        {Event::note(62, 100, 1), Event::note(65, 100, 1)},
        {},
        {Event::note(72, 100, 1)}
    };

    auto r = RunResult<Event>::dummy(c);
    REQUIRE_THAT(r, mms::voice_equals(0, NC::on(60)));
    REQUIRE_THAT(r, mms::voice_equals(1, {NC::on(62), NC::on(65)}));
    REQUIRE_THAT(r, mms::voice_equals(2, NC::empty()));
    REQUIRE_THAT(r, mms::voice_equals(3, NC::on(72)));
    //
    // // Negative
    REQUIRE_THAT(r, !mms::voice_equals(0, NC::on(62)));
    REQUIRE_THAT(r, !mms::voice_equals(0, NC::empty()));
    REQUIRE_THAT(r, !mms::voice_equals(0, {NC::on(60), NC::on(62)}));
}