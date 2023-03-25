#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../src/transport.h"

#include <chrono>
#include <thread>
#include <iostream>

TEST_CASE("dummy test2", "[dummytag1, dummytag2]") {
    REQUIRE(1 == 1);
}


TEST_CASE("dummy test2", "[dummytag3]") {
    REQUIRE(0 < 1);
}

TEST_CASE("transport test", "[transport]") {
    Transport transport(TimePoint(0.0, 60.0, 0.0, Meter(4, 4)), true);

    REQUIRE_THAT(transport.get_current_time().get_tick(), Catch::Matchers::WithinAbs(0.0, 1e-5));
    REQUIRE_THAT(transport.get_current_time().get_tempo(), Catch::Matchers::WithinAbs(60.0, 1e-5));

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    REQUIRE_THAT(transport.get_current_time().get_tick(), Catch::Matchers::WithinAbs(1.0, 0.1));

    transport = Transport(TimePoint(0.0, 180.0, 0.0, Meter(4, 4)), true);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    REQUIRE_THAT(transport.get_current_time().get_tick(), Catch::Matchers::WithinAbs(3.0, 0.1));

    transport = Transport(TimePoint(0.0, 20.0, 0.0, Meter(4, 4)), true);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    REQUIRE_THAT(transport.get_current_time().get_tick(), Catch::Matchers::WithinAbs(1.0 / 3.0, 0.1));


}