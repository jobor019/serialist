
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/algo/temporal/transport.h"
#include "core/algo/temporal/time_point.h"

using namespace serialist;

TEST_CASE("Initial TimePoint") {
    auto t = TimePoint();
    REQUIRE_THAT(t.get_tick(), Catch::Matchers::WithinAbs(0.0, 1e-8));
    REQUIRE_THAT(t.get_absolute_beat(), Catch::Matchers::WithinAbs(0.0, 1e-8));
    REQUIRE_THAT(t.get_relative_beat(), Catch::Matchers::WithinAbs(0.0, 1e-8));
    REQUIRE_THAT(t.get_bar(), Catch::Matchers::WithinAbs(0.0, 1e-8));
}

TEST_CASE("TimePoint increment (4/4") {
    auto t = TimePoint();
    t.increment(1.0);
    REQUIRE_THAT(t.get_tick(), Catch::Matchers::WithinAbs(1.0, 1e-8));
    REQUIRE_THAT(t.get_absolute_beat(), Catch::Matchers::WithinAbs(1.0, 1e-8));
    REQUIRE_THAT(t.get_relative_beat(), Catch::Matchers::WithinAbs(1.0, 1e-8));
    REQUIRE_THAT(t.get_bar(), Catch::Matchers::WithinAbs(0.25, 1e-8));

    t.increment(4.0);
    REQUIRE_THAT(t.get_tick(), Catch::Matchers::WithinAbs(5.0, 1e-8));
    REQUIRE_THAT(t.get_absolute_beat(), Catch::Matchers::WithinAbs(5.0, 1e-8));
    REQUIRE_THAT(t.get_relative_beat(), Catch::Matchers::WithinAbs(1.0, 1e-8));
    REQUIRE_THAT(t.get_bar(), Catch::Matchers::WithinAbs(1.25, 1e-8));
}

