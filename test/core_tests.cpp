#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../src/transport.h"
#include "../src/phasor.h"


#include <chrono>
#include <thread>
#include <iostream>
#include <cmath>


TEST_CASE("dummy test", "[dummytag1, dummytag2]") {
    REQUIRE(1 == 1);
}

TEST_CASE("phasor stepped", "[phasor]") {

//    std::cout << std::fixed;
//    std::cout << std::setprecision(16);


    SECTION("unity phasor") {
        double gain = 0.1;
        double max = 1.0;
        Phasor p{gain, max, 0.0, Phasor::Mode::stepped, 0.0};

        double x;
        for (int i = 1; i < 100; ++i) {
            x = p.process(0);
            REQUIRE_THAT(x, Catch::Matchers::WithinAbs(std::fmod(gain * i, max), 1e-8));
            REQUIRE(x < max);
        }
    }


    SECTION("Non-unity maximum") {
        double gain = 0.001;
        double max = 4.5;
        Phasor p{gain, max, 0.0, Phasor::Mode::stepped, 0.0};

        double x;
        for (int i = 1; i < 100; ++i) {
            x = p.process(0);
            REQUIRE_THAT(x, Catch::Matchers::WithinAbs(std::fmod(gain * i, max), 1e-8));
            REQUIRE(x < max);
        }
    }

    SECTION("Negative step size") {
        double gain = -0.1;
        double max = 1.0;
        Phasor p{gain, max, 0.0, Phasor::Mode::stepped, 0.0};


        double x;
        double y = 0;
        for (int i = 1; i < 100; ++i) {
            x = p.process(0);
            y += gain;
            if (y < 0) {
                y += max;
            }
            REQUIRE_THAT(x, Catch::Matchers::WithinAbs(y, 1e-8));
            REQUIRE(x < max);
            REQUIRE(x >= 0);
        }
    }

    SECTION("Variable step size") {
        double max = 1.0;
        Phasor p{0.1, max, 0.0, Phasor::Mode::stepped, 0.0};
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.1, 1e-8));
        p.set_step_size(0.2);
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.3, 1e-8));
        p.set_step_size(0.7);
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.0, 1e-8));
    }

    SECTION("Variable phase") {
        double max = 1.0;
        Phasor p{0.1, max, 0.5, Phasor::Mode::stepped, 0.0};
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.6, 1e-8));
        p.set_phase(0.2);
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.3, 1e-8));
        p.set_phase(0.9);
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.0, 1e-8));
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.1, 1e-8));
    }


}


TEST_CASE("transport test", "[transport]") {
    Transport transport(TimePoint(0.0, 60.0, 0.0, Meter(4, 4)), true);

    REQUIRE_THAT(transport.update_time().get_tick(), Catch::Matchers::WithinAbs(0.0, 1e-5));
    REQUIRE_THAT(transport.update_time().get_tempo(), Catch::Matchers::WithinAbs(60.0, 1e-5));

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    REQUIRE_THAT(transport.update_time().get_tick(), Catch::Matchers::WithinAbs(1.0, 0.1));

    transport = Transport(TimePoint(0.0, 180.0, 0.0, Meter(4, 4)), true);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    REQUIRE_THAT(transport.update_time().get_tick(), Catch::Matchers::WithinAbs(3.0, 0.1));

    transport = Transport(TimePoint(0.0, 20.0, 0.0, Meter(4, 4)), true);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    REQUIRE_THAT(transport.update_time().get_tick(), Catch::Matchers::WithinAbs(1.0 / 3.0, 0.1));


}