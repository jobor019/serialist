

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/policies/policies.h"
#include "node_runner.h"
#include "core/generatives/patternizer.h"

#include "generators.h"
#include "matchers/m1m.h"

using namespace serialist;
using namespace serialist::test;

using Mode = PatternizerIntWrapper<>::Mode;

TEST_CASE("Patternizer: ctor (temp))", "[patternizer]") {
    PatternizerIntWrapper<> w;
}

TEST_CASE("Patternizer: basic operation (temp)", "[patternizer]") {
    PatternizerIntWrapper<> w;

    w.chord.set_values(Voices<int>::singular({10, 11, 12, 13}));

    w.mode.set_values(Mode::from_bottom);
    w.cursor.set_values(Voices<double>::singular({0, 2}));
    w.cursor_is_index.set_value(true);

    w.trigger.set_values(Trigger::pulse_on());

    NodeRunner runner{&w.patternizer_mode};

    auto r = runner.step();
    // TODO: Implement proper matchers
    REQUIRE_THAT(r, m1m::size<Facet>(2));

    REQUIRE((*r.voices())[0][0] == 10);
    REQUIRE((*r.voices())[0][1] == 11);
}
