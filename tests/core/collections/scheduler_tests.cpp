#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/collections/scheduler.h"

using namespace serialist;

TEST_CASE("Scheduler tests", "[scheduler]") {
    // TODO: Implement proper tests
    Scheduler<std::string, double> scheduler;

    scheduler.schedule("first", 1.0);
    scheduler.schedule("third", 3.0);
    scheduler.schedule("second", 2.0);

//    REQUIRE(scheduler.size() == 3);
    REQUIRE(scheduler.empty() == false);

    scheduler.poll(1.0);
    scheduler.clear();
}
