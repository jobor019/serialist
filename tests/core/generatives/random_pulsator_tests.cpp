//
//#include <catch2/catch_test_macros.hpp>
//#include <catch2/matchers/catch_matchers_floating_point.hpp>
//#include "core/generatives/random_pulsator.h"
//
//TEST_CASE("RandomPulsator valid pulses") {
//    auto pulsator = RandomPulsator(0.25, 1.0, 1.0, 0);
//
//    auto step = 0.01;
//
//    std::optional<Trigger> last_trigger = std::nullopt;
//
//    TimePoint t;
//    for (std::size_t i = 0; i < 1000; ++i) {
//        if (auto triggers = pulsator.process(t.get_tick()); !triggers.empty()) {
//            if (last_trigger.has_value()) {
//
//            }
//
//            std::cout << "time: " << t.get_tick() << std::endl;
//            triggers.as_type<int>().print();
//        }
//        t.increment(step);
//    }
//}
//
//TEST_CASE("RandomPulsatorWrapper") {
//    RandomPulsatorWrapper<double> wrapper;
//
//    auto& pulsator = wrapper.random_pulsator;
//
//    auto step = 0.01;
//    TimePoint t;
//    for (std::size_t i = 0; i < 1000; ++i) {
//        pulsator.update_time(t);
//        if (auto triggers = pulsator.process(); !triggers.is_empty_like()) {
//            std::cout << "time: " << t.get_tick() << std::endl;
//            triggers.as_type<int>().print();
//        }
//        t.increment(step);
//    }
//
//    std::cout << "final tick: " << t.get_tick() << std::endl;
//}
//
//TEST_CASE("RandomPulsatorWrapper: updating values") {
//    RandomPulsatorWrapper<double> wrapper;
//
//    auto& pulsator = wrapper.random_pulsator;
//
//    auto values = Vec{Voices<double>::singular(2.0), Voices<double>::singular(3.0)};
//
//    auto step = 0.01;
//    TimePoint t;
//    for (std::size_t i = 0; i < 1000; ++i) {
//        wrapper.upper_bound.set_values(values[i % 2]);
//
//        pulsator.update_time(t);
//        if (auto triggers = pulsator.process(); !triggers.is_empty_like()) {
//            std::cout << "time: " << t.get_tick() << std::endl;
//            triggers.as_type<int>().print();
//        }
//        t.increment(step);
//    }
//}
//
