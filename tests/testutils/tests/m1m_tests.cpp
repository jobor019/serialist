#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "matchers/matchers_common.h"
#include "matchers/m1m.h"

using namespace serialist::test;

TEST_CASE("testutils::m11 trigger comparisons", "[testutils][m11]") {
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
        
        // Sequence with NO_ID triggers is sorted if other conditions are met
        REQUIRE_THAT(RunResult<Trigger>::dummy(Vec{
            Trigger::without_id(Trigger::Type::pulse_on)
            , Trigger::with_manual_id(Trigger::Type::pulse_on, 1)
            , Trigger::without_id(Trigger::Type::pulse_off)
            , Trigger::with_manual_id(Trigger::Type::pulse_off, 1)
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
