#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/temporal/transport.h"
#include "core/types/time_point.h"
#include "matchers/matchers_common.h"

using namespace serialist;
using namespace serialist::test;

TEST_CASE("TimePoint: TimePointMatcher validity", "[time_point]") {
    auto t1 = TimePoint();
    auto t2 = TimePoint(1.0);
    REQUIRE_THAT(t1, TimePointMatcher(t1));
    REQUIRE_THAT(t1, !TimePointMatcher(t2));

    SECTION("Positive cases") {
        REQUIRE_THAT(t1, TimePointMatcher(0.0, 0.0, 0.0, 0.0));
        REQUIRE_THAT(t1, TimePointMatcher().with_tick(0.0));
        REQUIRE_THAT(t1, TimePointMatcher().with_relative_beat(0.0));
        REQUIRE_THAT(t1, TimePointMatcher().with_absolute_beat(0.0));
        REQUIRE_THAT(t1, TimePointMatcher().with_bar(0.0));
        REQUIRE_THAT(t1, TimePointMatcher().with_meter(Meter{4, 4}));
        REQUIRE_THAT(t1, TimePointMatcher::zero());
    }

    SECTION("Negative cases") {
        REQUIRE_THAT(t1, !TimePointMatcher().with_tick(1.0));
        REQUIRE_THAT(t1, !TimePointMatcher().with_relative_beat(1.0));
        REQUIRE_THAT(t1, !TimePointMatcher().with_absolute_beat(1.0));
        REQUIRE_THAT(t1, !TimePointMatcher().with_bar(1.0));
        REQUIRE_THAT(t1, !TimePointMatcher().with_meter(Meter{5, 4}));

        REQUIRE_THAT(t2, !TimePointMatcher::zero());
    }
}


// ==============================================================================================

TEST_CASE("TimePoint: Initial TimePoint is 0.0", "[time_point]") {
    auto t = TimePoint();
    REQUIRE_THAT(t, TimePointMatcher(0.0, 0.0, 0.0, 0.0));
    REQUIRE_THAT(t, TimePointMatcher::zero()); // equivalent
}


TEST_CASE("TimePoint: increment by tick in fixed meter", "[time_point]") {
    SECTION("Beat denominator = tick size (quarter note)") {
        auto t = TimePoint();
        t.increment(1.0);
        REQUIRE_THAT(t, TimePointMatcher(1.0, 1.0, 1.0, 0.25));

        t.increment(4.0);
        REQUIRE_THAT(t, TimePointMatcher(5.0, 5.0, 1.0, 1.25));
    }

    SECTION("Beat denominator != tick size (eight note)") {
        auto t = TimePoint().with_meter(Meter{4, 8});
        t.increment(1.0);
        REQUIRE_THAT(t, TimePointMatcher(1.0, 2.0, 2.0, 0.5));
        t.increment(1.0);
        REQUIRE_THAT(t, TimePointMatcher(2.0, 4.0, 0.0, 1.0));
    }
}


TEST_CASE("TimePoint: increment by beat in fixed meter", "[time_point]") {
    SECTION("Beat denominator = tick size (quarter note)") {
        auto t = TimePoint();
        t.increment(DomainDuration::beats(1.0));
        REQUIRE_THAT(t, TimePointMatcher(1.0, 1.0, 1.0, 0.25));

        t.increment(DomainDuration::beats(4.0));
        REQUIRE_THAT(t, TimePointMatcher(5.0, 5.0, 1.0, 1.25));
    }

    SECTION("Beat denominator != tick size (eight note)") {
        auto t = TimePoint().with_meter(Meter{4, 8});
        t.increment(DomainDuration::beats(1.0));
        REQUIRE_THAT(t, TimePointMatcher(0.5, 1.0, 1.0, 0.25));
        t.increment(DomainDuration::beats(3.0));
        REQUIRE_THAT(t, TimePointMatcher(2.0, 4.0, 0.0, 1.0));
    }
}


TEST_CASE("TimePoint: increment by bar in fixed meter", "[time_point]") {
    SECTION("Beat denominator = tick size (quarter note)") {
        auto t = TimePoint();
        t.increment(DomainDuration::bars(0.25));
        REQUIRE_THAT(t, TimePointMatcher(1.0, 1.0, 1.0, 0.25));

        t.increment(DomainDuration::bars(1.0));
        REQUIRE_THAT(t, TimePointMatcher(5.0, 5.0, 1.0, 1.25));
    }

    SECTION("Beat denominator != tick size (eight note)") {
        auto t = TimePoint().with_meter(Meter{4, 8});
        t.increment(DomainDuration::bars(0.25));
        REQUIRE_THAT(t, TimePointMatcher(0.5, 1.0, 1.0, 0.25));
        t.increment(DomainDuration::bars(0.75));
        REQUIRE_THAT(t, TimePointMatcher(2.0, 4.0, 0.0, 1.0));
    }
}


TEST_CASE("TimePoint: Ticks to next bar is not subject to rounding errors", "[time_point]") {
    auto [start_tick, expected_next_bar] = GENERATE(
        std::pair{0.0, 0.0} // When starting exactly at a bar, the value returned should be the current bar, not next
        , std::pair{4.0, 1.0} // same
        , std::pair{4'000'000.0, 1'000'000.0} // same
        , std::pair{0.1, 1.0}
        , std::pair{3.0, 1.0}
        , std::pair{3.999, 1.0}
        , std::pair{103, 26.0}
        , std::pair{3'999'999'999.9999, 1'000'000'000.0}
    );

    auto t = TimePoint(start_tick).with_meter(Meter{4, 4});
    t += t.ticks_to_next_bar();
    REQUIRE_THAT(t, TimePointMatcher().with_bar(expected_next_bar)); // Approximately equal to `expected_next_bar`
    REQUIRE(t.get_bar() >= expected_next_bar);                       // Not subject to rounding errors
}


TEST_CASE("TimePoint: beat consistency in meter change", "[time_point]") {
    SECTION("Meter change occurs exactly at current tick") {
        auto t = TimePoint().with_meter(Meter{4, 4});
        bool rc = t.increment_with_meter_change(1.0, Meter{4, 8});
        REQUIRE(rc);
        REQUIRE_THAT(t, TimePointMatcher(1.0, 2.0, 2.0, 0.5).with_meter(Meter{4, 8}));
    }

    SECTION("Meter change occurs exactly at next bar") {
        auto t = TimePoint(3.0).with_meter(Meter{4, 4});
        bool rc = t.increment_with_meter_change(t.ticks_to_next_bar(), Meter{4, 8});
        REQUIRE(rc);
        REQUIRE_THAT(t, TimePointMatcher(4.0, 4.0, 0.0, 1.0).with_meter(Meter{4, 8}));
    }

    SECTION("Meter change occurs mid-way through tick increment") {
        auto t = TimePoint(3.0).with_meter(Meter{4, 4});
        REQUIRE_THAT(t, TimePointMatcher(3.0, 3.0, 3.0, 0.75));

        // 1.0 ticks in old bar (1.0 ticks = 1.0 beats = 0.25 bars),
        // 1.0 ticks in new bar (1.0 ticks = 2.0 beats = 0.5 bars)
        bool rc = t.increment_with_meter_change(2.0, Meter{4, 8});
        REQUIRE(rc);
        REQUIRE_THAT(t, TimePointMatcher(5.0, 6.0, 2.0, 1.5).with_meter(Meter{4, 8}));
    }

    SECTION("Insufficient ticks for meter change") {
        auto t = TimePoint(3.0).with_meter(Meter{4, 4});
        REQUIRE_THAT(t, TimePointMatcher().with_bar(0.75));

        bool rc = t.increment_with_meter_change(0.5, Meter{4, 8});
        REQUIRE(!rc);
        REQUIRE_THAT(t, TimePointMatcher().with_bar(0.875).with_meter(Meter{4, 4}));
    }
}
