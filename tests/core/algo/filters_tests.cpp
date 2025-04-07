
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <iostream>
#include "core/temporal/filters.h"
#include "core/collections/vec.h"
#include "core/algo/random/random.h"
#include "core/utility/math.h"

using namespace serialist;

TEST_CASE("FilterSmoo") {
    FilterSmoo s;
    Random rnd(0);

    std::size_t num_values = 1000;

    auto step = 0.01;

    auto ticks = Vec<TimePoint>::allocated(num_values);
    ticks.append(TimePoint(rnd.next()));

    for (std::size_t i = 1; i < num_values; ++i) {
        ticks.append(ticks[-1].incremented(rnd.next() * step));
    }

    auto freq = 0.5;
    auto phasor = ticks.cloned().as_type<double>([&freq](auto t) {
        return utils::modulo(t.get_tick(), 1 / freq) * freq;
    });

    auto square_wave = phasor.cloned().map([](auto x) { return static_cast<double>(x < 0.5); });

    SECTION("Stateless") {
        s.set_tau(DomainDuration{0.1, DomainType::ticks});
        auto y = Vec<double>::zeros(ticks.size());
        for (std::size_t i = 0; i < ticks.size(); ++i) {
            y[i] = s.process(ticks[i], square_wave[i]);
        }

        // std::cout << "ticks = ";
        // ticks.as_type<double>([](auto t) { return t.get_tick(); }).print();

        // std::cout << "x = ";
        // square_wave.print();

        // std::cout << "y = ";
        // y.print();
    }

//    SECTION("With setter") {
//        auto y = Vec<double>::zeros(ticks.size());
//        for (std::size_t i = 0; i < ticks.size(); ++i) {
//            y[i] = s.process(square_wave[i], ticks[i], 0.1, false);
//        }
//
//        std::cout << "ticks = ";
//        ticks.print();
//
//        std::cout << "x = ";
//        square_wave.print();
//
//        std::cout << "y = ";
//        y.print();
//    }

}

TEST_CASE("FilterSmoo stateless") {
    FilterSmoo s;
}
