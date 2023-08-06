#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "oscillator.h"


TEST_CASE("Oscillator") {
    auto um = juce::UndoManager();
    auto handler = ParameterHandler(um);
    auto osc = Oscillator("hehe", handler);

    auto t = TimePoint();

    auto s = osc.process(t);

    for (auto& e : s.vector()) {
        for (auto& ee : e.vector()) {
            std::cout << ee << "\n";
        }
    }
}