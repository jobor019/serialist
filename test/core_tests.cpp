#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../src/transport.h"
#include "../src/phasor.h"
#include "../src/oscillator.h"
#include "../src/mapping.h"
#include "../src/generation_graph.h"
#include "../src/looper.h"
#include "../src/scheduler.h"


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
        for (int i = 0; i < 100; ++i) {
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
        for (int i = 0; i < 100; ++i) {
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
        for (int i = 0; i < 100; ++i) {
            x = p.process(0);
            if (y < 0) {
                y += max;
            }
            REQUIRE_THAT(x, Catch::Matchers::WithinAbs(y, 1e-8));
            REQUIRE(x < max);
            REQUIRE(x >= 0);
            y += gain;
        }
    }

    SECTION("Variable step size") {
        double max = 1.0;
        Phasor p{0.1, max, 0.0, Phasor::Mode::stepped, 0.0};
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.0, 1e-8));
        p.set_step_size(0.2);
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.2, 1e-8));
        p.set_step_size(0.8);
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.0, 1e-8));
    }

    SECTION("Variable phase") {
        double max = 1.0;
        Phasor p{0.1, max, 0.5, Phasor::Mode::stepped, 0.0};
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.5, 1e-8));
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.6, 1e-8));
        p.set_phase(0.2);
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.2, 1e-8));
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.3, 1e-8));
        p.set_phase(0.9);
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.9, 1e-8));
        REQUIRE_THAT(p.process(0), Catch::Matchers::WithinAbs(0.0, 1e-8));
    }
}


TEST_CASE("generation graph", "[generation]") {
//    SimplisticMidiGraphV1 graph;
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
        Mapping<int> m{{  1}
                       , {2, 3}};
        std::cout << m.get(0)[0] << "\n";
        std::cout << m.get(1)[0] << "\n";
    }

    SECTION("mapping from vector") {
        std::vector<int> v{1, 2, 3};
        Mapping<int> m(v);
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


TEST_CASE("stepped looper", "[generation]") {
    Looper<int> looper{Mapping<int>{{  60}
                                    , {62}
                                    , {64}
                                    , {67}
                                    , {68}}, 1.0, 0.0, Phasor::Mode::stepped};

    SECTION("uniform integer looper") {
        REQUIRE(looper.process(TimePoint{}).at(0) == 60);
        REQUIRE(looper.process(TimePoint{}).at(0) == 62);
        REQUIRE(looper.process(TimePoint{}).at(0) == 64);
        REQUIRE(looper.process(TimePoint{}).at(0) == 67);
        REQUIRE(looper.process(TimePoint{}).at(0) == 68);
        REQUIRE(looper.process(TimePoint{}).at(0) == 60);
        REQUIRE(looper.process(TimePoint{}).at(0) == 62);
    }

    SECTION("double step looper") {
        looper.set_step_size(2.0);
        REQUIRE(looper.process(TimePoint{}).at(0) == 60);
        REQUIRE(looper.process(TimePoint{}).at(0) == 64);
        REQUIRE(looper.process(TimePoint{}).at(0) == 68);
        REQUIRE(looper.process(TimePoint{}).at(0) == 62);
        REQUIRE(looper.process(TimePoint{}).at(0) == 67);
    }

    SECTION("negative looper") {
        looper.set_step_size(-1.0);
        REQUIRE(looper.process(TimePoint{}).at(0) == 60);
        REQUIRE(looper.process(TimePoint{}).at(0) == 68);
        REQUIRE(looper.process(TimePoint{}).at(0) == 67);
        REQUIRE(looper.process(TimePoint{}).at(0) == 64);
        REQUIRE(looper.process(TimePoint{}).at(0) == 62);
        REQUIRE(looper.process(TimePoint{}).at(0) == 60);
        REQUIRE(looper.process(TimePoint{}).at(0) == 68);
    }

    SECTION("euclidean looper") {
        looper.set_step_size(1.7);
        REQUIRE(looper.process(TimePoint{}).at(0) == 60);  // 0   ~ 0
        REQUIRE(looper.process(TimePoint{}).at(0) == 62);  // 1.7 ~ 1
        REQUIRE(looper.process(TimePoint{}).at(0) == 67);  // 3.4 ~ 3
        REQUIRE(looper.process(TimePoint{}).at(0) == 60);  // 5.1 ~ 0
        REQUIRE(looper.process(TimePoint{}).at(0) == 62);  // 6.8 ~ 1
        REQUIRE(looper.process(TimePoint{}).at(0) == 67);  // 8.5 ~ 3
    }

    SECTION("mid-way add") {
        REQUIRE(looper.process(TimePoint{}).at(0) == 60);
        REQUIRE(looper.process(TimePoint{}).at(0) == 62);
        REQUIRE(looper.process(TimePoint{}).at(0) == 64);
        looper.add(MapElement<int>(72), 0);
        REQUIRE(looper.process(TimePoint{}).at(0) == 67);
        REQUIRE(looper.process(TimePoint{}).at(0) == 68);
        REQUIRE(looper.process(TimePoint{}).at(0) == 72);
        REQUIRE(looper.process(TimePoint{}).at(0) == 60);
    }

    SECTION("mid-way replace and empty mapping") {
        REQUIRE(looper.process(TimePoint{}).at(0) == 60);
        REQUIRE(looper.process(TimePoint{}).at(0) == 62);
        REQUIRE(looper.process(TimePoint{}).at(0) == 64);
        looper.set_mapping({});
        REQUIRE(looper.process(TimePoint{}).empty());
        REQUIRE(looper.process(TimePoint{}).empty());
        REQUIRE(looper.process(TimePoint{}).empty());
        std::vector<int> new_elements = {60, 70, 80};
        looper.set_mapping(Mapping<int>(new_elements));
        std::vector<int> output;
        output.emplace_back(looper.process(TimePoint{}).at(0));
        output.emplace_back(looper.process(TimePoint{}).at(0));
        output.emplace_back(looper.process(TimePoint{}).at(0));

        REQUIRE(std::all_of(new_elements.begin(), new_elements.end(), [&output](int i) {
            return std::find(output.begin(), output.end(), i) != output.end();
        }));
    }

}


TEST_CASE("scheduler", "[scheduling]") {
    Scheduler s;
    s.add_event(std::make_unique<MidiEvent>(1.0, 7200, 127, 1));
    s.add_event(std::make_unique<TriggerEvent>(3.0));
    s.add_event(std::make_unique<TriggerEvent>(2.0));

    auto v = s.get_events(2.4);

    REQUIRE(s.size() == 1);

    for (auto& e: v) {
        std::cout << e->get_time() << "\n";
    }

    // TODO
}


