#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../src/transport.h"
#include "../src/phasor.h"
#include "../src/oscillator.h"
#include "../src/map.h"
#include "../src/generation_graph.h"


#include <chrono>
#include <thread>
#include <iostream>
#include <cmath>


TEST_CASE("dummy test", "[dummytag1, dummytag2]") {
    REQUIRE(1 == 1);
}

TEST_CASE("m_phasor stepped", "[m_phasor]") {

//    std::cout << std::fixed;
//    std::cout << std::setprecision(16);


    SECTION("unity m_phasor") {
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


TEST_CASE("generation graph", "[generation]") {
    SimplisticMidiGraphV1 graph;
}

TEST_CASE("oscillators", "[generation]") {
    SECTION("cos") {
        Cosine o;

        REQUIRE_THAT(o.process(0), Catch::Matchers::WithinAbs(1, 1e-8));
        REQUIRE_THAT(o.process(0.25), Catch::Matchers::WithinAbs(0.5, 1e-8));
        REQUIRE_THAT(o.process(0.5), Catch::Matchers::WithinAbs(0, 1e-8));
        REQUIRE_THAT(o.process(0.75), Catch::Matchers::WithinAbs(0.5, 1e-8));
        REQUIRE_THAT(o.process(1), Catch::Matchers::WithinAbs(1, 1e-8));
    }

    SECTION("square") {
        Square o{0.5};

        REQUIRE_THAT(o.process(0), Catch::Matchers::WithinAbs(1, 1e-8));
        REQUIRE_THAT(o.process(0.5), Catch::Matchers::WithinAbs(1, 1e-8));
        REQUIRE_THAT(o.process(0.500001), Catch::Matchers::WithinAbs(0, 1e-8));
        REQUIRE_THAT(o.process(1), Catch::Matchers::WithinAbs(0, 1e-8));

        o.set_duty(0.0);
        REQUIRE_THAT(o.process(0), Catch::Matchers::WithinAbs(1, 1e-8));
        REQUIRE_THAT(o.process(0.000001), Catch::Matchers::WithinAbs(0, 1e-8));
        REQUIRE_THAT(o.process(0.5), Catch::Matchers::WithinAbs(0, 1e-8));
        REQUIRE_THAT(o.process(1), Catch::Matchers::WithinAbs(0, 1e-8));

        o.set_duty(0.8);
        REQUIRE_THAT(o.process(0), Catch::Matchers::WithinAbs(1, 1e-8));
        REQUIRE_THAT(o.process(0.8), Catch::Matchers::WithinAbs(1, 1e-8));
        REQUIRE_THAT(o.process(0.800001), Catch::Matchers::WithinAbs(0, 1e-8));
        REQUIRE_THAT(o.process(1), Catch::Matchers::WithinAbs(0, 1e-8));

        o.set_duty(1.0);
        REQUIRE_THAT(o.process(0), Catch::Matchers::WithinAbs(1, 1e-8));
        REQUIRE_THAT(o.process(1), Catch::Matchers::WithinAbs(1, 1e-8));
    }

    SECTION("triangle") {
        // TODO
    }
}

TEST_CASE("mappings") {
    SECTION("map elements") {
        SECTION("empty") {
            MapElement<int> m{};
        }

        SECTION("one elem") {
            MapElement<int> m{123};
            REQUIRE(m.temp_first() == 123);
        }

        SECTION("several") {
            MapElement<int> m{123, 234, 888};
            REQUIRE(m.temp_first() == 123);
        }
    }

    SECTION("m_mapping") {
        Mapping<int> m{{1}, {2, 3}};
        std::cout << m.at(0).temp_first() << "\n";
        std::cout << m.at(1).temp_first() << "\n";
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

TEST_CASE("generation graph", "[generation, integration]") {
    SimplisticMidiGraphV1 m;

}
