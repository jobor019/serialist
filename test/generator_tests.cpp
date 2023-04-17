#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../src/generator.h"

#include <iostream>

TEST_CASE("Trivial Generator") {
    auto step_size = 0.1;
    Generator<double> generator{step_size};

    for (int i = 0; i < 10; ++i) {
        auto elements = generator.process(TimePoint());
        REQUIRE(elements.size() == 1);
        REQUIRE_THAT(elements[0], Catch::Matchers::WithinAbs(i * step_size, 1e-8));
    }
    auto elements = generator.process(TimePoint());
    REQUIRE(elements.size() == 1);
    REQUIRE_THAT(elements[0], Catch::Matchers::WithinAbs(0.0, 1e-8));
}


// ==============================================================================================

TEST_CASE("Generator as Looper") {
    Mapping<int> mapping(std::vector{0, 2, 4, 5, 7, 9, 11});

    SECTION("Linear forward-looping") {
        auto step_size = 1.0 / static_cast<double>(mapping.size());

        Generator<int> generator{step_size
                                 , 0.0
                                 , Phasor::Mode::stepped
                                 , std::make_unique<Identity>()
                                 , std::make_unique<Mapping<int>>(mapping)};

        for (int i = 0; i < 100; ++i) {
            REQUIRE(generator.process(TimePoint()) == mapping.at(static_cast<std::size_t>(i) % mapping.size()));
        }
    }

    SECTION("Linear backward-looping") {
        auto step_size = -1.0 / static_cast<double>(mapping.size());

        Generator<int> generator{step_size
                                 , 0.0

                                 , Phasor::Mode::stepped
                                 , std::make_unique<Identity>()
                                 , std::make_unique<Mapping<int>>(mapping)};

        for (long i = 0; i < 100; ++i) {
            REQUIRE(generator.process(TimePoint()) ==
                    mapping.at(static_cast<std::size_t>(utils::modulo(-i, static_cast<long>(mapping.size())))));
        }
    }
}

