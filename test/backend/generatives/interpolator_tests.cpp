#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/generatives/interpolator.h"
#include "time_point.h"


TEST_CASE("Test Interpolator: Continue (Integral)") {
    Voices<int> corpus{{  0, 2}
                       , {4}
                       , {5}
                       , {7, 9, 11}};

    using Interp = Interpolator<int>;

    auto strategy = Interp::Continue{12};


    SECTION("Test position 0.0") {
        auto result = Interp::process(0.0, corpus, strategy);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 2);
    }

    SECTION("Test get with position 1.0") {
        auto result = Interp::process(1.0, corpus, strategy);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 12);
        REQUIRE(result[1] == 14);
    }

    SECTION("Test get with position 0.5") {
        auto result = Interp::process(0.5, corpus, strategy);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with position 2.5") {
        auto result = Interp::process(2.5, corpus, strategy);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5 + 12 * 2);
    }

    SECTION("Test get with position -1.5") {
        auto result = Interp::process(-1.5, corpus, strategy);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5 - 12 * 2);
    }

    SECTION("Test get with empty mapping") {
        auto empty_corpus = Voices<int>::empty_like();
        auto result = Interp::process(1.0, empty_corpus, strategy);
        REQUIRE(result.empty());
    }

    SECTION("Test get with mixed mappings") {
        auto mixed_corpus = Voices{Voice<int>{0, 2}, Voice<int>(), Voice<int>{8, 10}};
        auto result = Interp::process(0.0, mixed_corpus, strategy);
        REQUIRE(result == Voice<int>{0, 2});
        result = Interp::process(0.4, mixed_corpus, strategy);
        REQUIRE(result.empty());
        result = Interp::process(0.8, mixed_corpus, strategy);
        REQUIRE(result == Voice<int>{8, 10});
    }

    SECTION("Immutability") {
        auto result = Interp::process(0.0, corpus, strategy);
        REQUIRE(result.size() == 2);
        result[0] = -1;
        result.append(-2);
        REQUIRE(corpus[0] == Voice<int>{0, 2});
    }
}


TEST_CASE("Test Interpolator: Continue (floating)") {
    Voices<double> corpus{{  0, 2}
                          , {4}
                          , {5}
                          , {7, 9, 11}};

    using Interp = Interpolator<double>;

    auto strategy = Interp::Continue{12.0};


    SECTION("Test position 0.0") {
        auto result = Interp::process(0.0, corpus, strategy);
        REQUIRE(result.size() == 2);
        REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(0, 1e-6));
        REQUIRE_THAT(result[1], Catch::Matchers::WithinAbs(2, 1e-6));
    }

    SECTION("Test get with position 1.0") {
        auto result = Interp::process(1.0, corpus, strategy);
        REQUIRE(result.size() == 2);
        REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(12, 1e-6));
        REQUIRE_THAT(result[1], Catch::Matchers::WithinAbs(14, 1e-6));
    }

    SECTION("Test get with position 1.15") {
        auto result = Interp::process(1.15, corpus, strategy);
        REQUIRE(result.size() == 2);
        REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(12, 1e-6));
        REQUIRE_THAT(result[1], Catch::Matchers::WithinAbs(14, 1e-6));
    }

    SECTION("Test get with position 2.5") {
        auto result = Interp::process(2.5, corpus, strategy);
        REQUIRE(result.size() == 1);
        REQUIRE_THAT(result[0], Catch::Matchers::WithinAbs(5 + 12 * 2, 1e-6));
    }
}


// ==============================================================================================

TEST_CASE("Test Interpolator: Modulo (Integral)") {
    Voices<int> corpus{{  0, 2}
                       , {4}
                       , {5}
                       , {7, 9, 11}};

    using Interp = Interpolator<int>;

    auto strategy = Interp::Modulo{};

    SECTION("Test position 0.0") {
        auto result = Interp::process(0.0, corpus, strategy);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 2);
    }

    SECTION("Test get with position 1.0") {
        auto result = Interp::process(1.0, corpus, strategy);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 2);
    }

    SECTION("Test get with position 0.5") {
        auto result = Interp::process(0.5, corpus, strategy);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with position 2.5") {
        auto result = Interp::process(2.5, corpus, strategy);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with position -1.5") {
        auto result = Interp::process(-1.5, corpus, strategy);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with empty mapping") {
        Voices<int> empty_corpus = Voices<int>::empty_like();
        auto result = Interp::process(2.25, empty_corpus, strategy);
        REQUIRE(result.empty());
    }

    SECTION("Immutability") {
        auto result = Interp::process(0, corpus, strategy);
        REQUIRE(result.size() == 2);
        result[0] = -1;
        result.append(-2);
        REQUIRE(corpus[0] == Voice<int>{0, 2});
    }
}

// ==============================================================================================

TEST_CASE("Test Interpolator: Clip (Integral)") {
    Voices<int> corpus{{  0, 2}
                       , {4}
                       , {5}
                       , {7, 9, 11}};

    using Interp = Interpolator<int>;

    auto strategy = Interp::Clip{};

    SECTION("Test position 0.0") {
        auto result = Interp::process(0.0, corpus, strategy);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 2);
    }

    SECTION("Test get with position 1.0") {
        auto result = Interp::process(1.0, corpus, strategy);
        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == 7);
        REQUIRE(result[1] == 9);
        REQUIRE(result[2] == 11);
    }

    SECTION("Test get with position 0.5") {
        auto result = Interp::process(0.5, corpus, strategy);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with position 2.5") {
        auto result = Interp::process(2.5, corpus, strategy);
        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == 7);
        REQUIRE(result[1] == 9);
        REQUIRE(result[2] == 11);
    }

    SECTION("Test get with position -1.5") {
        auto result = Interp::process(-1.5, corpus, strategy);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 2);
    }

    SECTION("Test get with empty mapping") {
        Voices<int> empty_corpus = Voices<int>::empty_like();
        auto result = Interp::process(2.25, empty_corpus, strategy);
        REQUIRE(result.empty());
    }

    SECTION("Immutability") {
        auto result = Interp::process(0, corpus, strategy);
        REQUIRE(result.size() == 2);
        result[0] = -1;
        result.append(-2);
        REQUIRE(corpus[0] == Voice<int>{0, 2});
    }
}

// ==============================================================================================

TEST_CASE("Test Interpolator: Pass (Integral)") {
    Voices<int> corpus{{  0, 2}
                       , {4}
                       , {5}
                       , {7, 9, 11}};

    using Interp = Interpolator<int>;

    auto strategy = Interp::Pass{};

    SECTION("Test position 0.0") {
        auto result = Interp::process(0.0, corpus, strategy);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 2);
    }

    SECTION("Test get with position 1.0") {
        auto result = Interp::process(1.0, corpus, strategy);
        REQUIRE(result.empty());
    }

    SECTION("Test get with position 0.5") {
        auto result = Interp::process(0.5, corpus, strategy);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with position 2.5") {
        auto result = Interp::process(2.5, corpus, strategy);
        REQUIRE(result.empty());
    }

    SECTION("Test get with position -1.5") {
        auto result = Interp::process(-1.5, corpus, strategy);
        REQUIRE(result.empty());
    }

    SECTION("Test get with empty mapping") {
        Voices<int> empty_corpus = Voices<int>::empty_like();
        auto result = Interp::process(2.25, empty_corpus, strategy);
        REQUIRE(result.empty());
    }

    SECTION("Immutability") {
        auto result = Interp::process(0, corpus, strategy);
        REQUIRE(result.size() == 2);
        result[0] = -1;
        result.append(-2);
        REQUIRE(corpus[0] == Voice<int>{0, 2});
    }
}



// ==============================================================================================


template<typename InterpolatorType>
void test_variable_size_interpolation(typename InterpolatorType::Strategy strategy) {
    for (std::size_t mapping_size = 1; mapping_size <= 1000; ++mapping_size) {
        auto corpus = Voices<int>::zeros(mapping_size);
        for (std::size_t i = 0; i < mapping_size; ++i) {
            corpus[i] = (Voice<int>{static_cast<int>(i)});
        }

        double position = 0;
        double increment = 1.0 / static_cast<double>(mapping_size);
        for (std::size_t i = 0; i < mapping_size; ++i) {
            REQUIRE(InterpolatorType::process(position, corpus, strategy) == corpus[i]);
            position += increment;
        }
    }
}


TEST_CASE("Interpolation using corpora with sizes between 1 and 1000 ") {
    using Interp = Interpolator<int>;

    SECTION("Continue") {
        auto strategy = Interp::Continue{0};
        test_variable_size_interpolation<Interp>(strategy);
    }

    SECTION("Modulo") {
        auto strategy = Interp::Modulo{};
        test_variable_size_interpolation<Interp>(strategy);
    }

    SECTION("Clip") {
        auto strategy = Interp::Clip{};
        test_variable_size_interpolation<Interp>(strategy);
    }

    SECTION("Pass") {
        auto strategy = Interp::Pass{};
        test_variable_size_interpolation<Interp>(strategy);
    }
}


// ==============================================================================================


TEST_CASE("InterpolatorWrapper") {
    InterpolatorWrapper<Facet, int, double> interp;

    interp.trigger.set_values(Trigger::pulse_on());

    auto corpus = Voices<int>{{0}, {1, 5}, {2, 6}, {3, 7}, {4}};
    interp.corpus.set_values(corpus);

    for (std::size_t i = 0; i < corpus.size() * 2; ++i) {
        interp.cursor.set_values(static_cast<double>(utils::modulo(i, corpus.size())) / static_cast<double>(corpus.size()));
        interp.interpolator.update_time(TimePoint());
        auto s  = interp.interpolator.process();
    }
}

TEST_CASE("InterpolatorAdapter") {
    ParameterHandler handler;
    InterpolationAdapter<Facet> strategy(InterpolationAdapter<Facet>::CLASS_NAME, handler, nullptr, nullptr);
}

