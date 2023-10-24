
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <iostream>
#include "core/algo/time/filters.h"
#include "core/collections/vec.h"
#include "core/algo/random.h"
#include "core/utility/math.h"

TEST_CASE("Smoo") {
    Smoo s;


    Random rnd(0);

    auto step = 0.01;

    auto ticks = rnd.nexts<double>(1000).multiply(step).cumsum();

    auto freq = 0.5;
    auto phasor = ticks.cloned().map([&freq](auto x) { return utils::modulo(x, 1/freq) * freq;});
    auto square_wave = phasor.cloned().map([](auto x) { return static_cast<double>(x < 0.5); });

    auto y = Vec<double>::zeros(ticks.size());
    for (std::size_t i = 0; i < ticks.size(); ++i) {
        y[i] = s.process(square_wave[i], ticks[i]);
    }

    std::cout << "ticks = ";
    ticks.print();

    std::cout << "x = ";
    square_wave.print();

    std::cout << "y = ";
    y.print();

}
