#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/policies/policies.h"
#include "core/generatives/interpolator.h"

#include "generators.h"
#include "node_runner.h"
#include "matchers/m1m.h"
#include "matchers/m11.h"

using namespace serialist;
using namespace serialist::test;

TEST_CASE("Interpolator: Continue Mode (integral)", "[interpolator]") {

    InterpolatorIntWrapper<> w;
    NodeRunner runner{&w.interpolator};

    auto& cursor = w.cursor;

    Voices<int> corpus{{  0, 2}
                       , {4}
                       , {5}
                       , {7, 9, 11}};

    w.corpus.set_values(corpus);
    w.mode.set_values(Index::Strategy::cont);
    w.octave.set_values(12.0);

    SECTION("position 0.0") {
        cursor.set_values(0.0);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{0, 2}));
    }

    SECTION("Test get with position 1.0") {
        cursor.set_values(1.0);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{12, 14}));
    }
    SECTION("Test get with position 0.5") {
        cursor.set_values(0.5);
        REQUIRE_THAT(runner.step(), m11::eqf(5));
    }

    SECTION("Test get with position 2.5") {
        cursor.set_values(2.5);
        REQUIRE_THAT(runner.step(), m11::eqf(5 + 12 * 2));
    }

    SECTION("Test get with position -1.5") {
        cursor.set_values(-1.5);
        REQUIRE_THAT(runner.step(), m11::eqf(5 - 12 * 2));
    }

    SECTION("Test get with empty mapping") {
        cursor.set_values(Voices<double>::empty_like());
        REQUIRE_THAT(runner.step(), m11::emptyf());
    }

    SECTION("Test get with mixed mappings") {
        Voices<int> mixed_corpus{{0, 2}, {}, {8, 10}};
        w.corpus.set_values(mixed_corpus);

        cursor.set_values(0.0);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{0, 2}));
        cursor.set_values(0.4);
        REQUIRE_THAT(runner.step(), m11::emptyf());
        cursor.set_values(0.8);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{8, 10}));
    }
}


TEST_CASE("Interpolator: Continue (floating)", "[interpolator]") {
    InterpolatorDoubleWrapper<> w;
    NodeRunner runner{&w.interpolator};

    auto& cursor = w.cursor;

    Voices<double> corpus{{  0, 2}
                         , {4}
                         , {5}
                         , {7, 9, 11}};

    w.corpus.set_values(corpus);
    w.mode.set_values(Index::Strategy::cont);
    w.octave.set_values(12.0);

    SECTION("position 0.0") {
        cursor.set_values(0.0);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{0, 2}));
    }

    SECTION("Test get with position 1.0") {
        cursor.set_values(1.0);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{12, 14}));
    }

    SECTION("Test get with position 1.15") {
        cursor.set_values(1.15);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{12, 14}));
    }

    SECTION("Test get with position 2.5") {
        cursor.set_values(2.5);
        REQUIRE_THAT(runner.step(), m11::eqf(5 + 12 * 2));
    }

    SECTION("Test get with empty mapping") {
        cursor.set_values(Voices<double>::empty_like());
        REQUIRE_THAT(runner.step(), m11::emptyf());
    }

    SECTION("Test get with mixed mappings") {
        Voices<double> mixed_corpus{{0, 2}, {}, {8, 10}};
        w.corpus.set_values(mixed_corpus);

        cursor.set_values(0.0);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{0, 2}));
        cursor.set_values(0.4);
        REQUIRE_THAT(runner.step(), m11::emptyf());
        cursor.set_values(0.8);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{8, 10}));
    }
}


// ==============================================================================================

TEST_CASE("Interpolator: Modulo (Integral)", "[interpolator]") {
    InterpolatorIntWrapper<> w;
    NodeRunner runner{&w.interpolator};

    auto& cursor = w.cursor;

    Voices<int> corpus{{  0, 2}
                       , {4}
                       , {5}
                       , {7, 9, 11}};

    w.corpus.set_values(corpus);
    w.mode.set_values(Index::Strategy::mod);
    w.octave.set_values(12.0);

    SECTION("position 0.0") {
        cursor.set_values(0.0);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{0, 2}));
    }

    SECTION("Test get with position 1.0") {
        cursor.set_values(1.0);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{0, 2}));
    }

    SECTION("Test get with position 0.5") {
        cursor.set_values(0.5);
        REQUIRE_THAT(runner.step(), m11::eqf(5));
    }

    SECTION("Test get with position 2.5") {
        cursor.set_values(2.5);
        REQUIRE_THAT(runner.step(), m11::eqf(5));
    }

    SECTION("Test get with position -1.5") {
        cursor.set_values(-1.5);
        REQUIRE_THAT(runner.step(), m11::eqf(5));
    }

    SECTION("Test get with empty mapping") {
        cursor.set_values(Voices<double>::empty_like());
        REQUIRE_THAT(runner.step(), m11::emptyf());
    }

    SECTION("Test get with mixed mappings") {
        Voices<int> mixed_corpus{{0, 2}, {}, {8, 10}};
        w.corpus.set_values(mixed_corpus);

        cursor.set_values(0.0);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{0, 2}));
        cursor.set_values(0.4);
        REQUIRE_THAT(runner.step(), m11::emptyf());
        cursor.set_values(0.8);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{8, 10}));
    }
}



// ==============================================================================================

TEST_CASE("Interpolator: Clip (Integral)", "[interpolator]") {
    InterpolatorIntWrapper<> w;
    NodeRunner runner{&w.interpolator};

    auto& cursor = w.cursor;

    Voices<int> corpus{{  0, 2}
                       , {4}
                       , {5}
                       , {7, 9, 11}};

    w.corpus.set_values(corpus);
    w.mode.set_values(Index::Strategy::clip);
    w.octave.set_values(12.0);

    SECTION("position 0.0") {
        cursor.set_values(0.0);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{0, 2}));
    }

    SECTION("Test get with position 1.0") {
        cursor.set_values(1.0);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{7, 9, 11}));
    }

    SECTION("Test get with position 0.5") {
        cursor.set_values(0.5);
        REQUIRE_THAT(runner.step(), m11::eqf(5));
    }

    SECTION("Test get with position 2.5") {
        cursor.set_values(2.5);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{7, 9, 11}));
    }

    SECTION("Test get with position -1.5") {
        cursor.set_values(-1.5);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{0, 2}));
    }

    SECTION("Test get with empty mapping") {
        cursor.set_values(Voices<double>::empty_like());
        REQUIRE_THAT(runner.step(), m11::emptyf());
    }

    SECTION("Test get with mixed mappings") {
        Voices<int> mixed_corpus{{0, 2}, {}, {8, 10}};
        w.corpus.set_values(mixed_corpus);

        cursor.set_values(0.0);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{0, 2}));
        cursor.set_values(0.4);
        REQUIRE_THAT(runner.step(), m11::emptyf());
        cursor.set_values(0.8);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{8, 10}));
    }
}

// ==============================================================================================

TEST_CASE("Interpolator: Pass (Integral)", "[interpolator]") {
    InterpolatorIntWrapper<> w;
    NodeRunner runner{&w.interpolator};

    auto& cursor = w.cursor;

    Voices<int> corpus{{  0, 2}
                       , {4}
                       , {5}
                       , {7, 9, 11}};

    w.corpus.set_values(corpus);
    w.mode.set_values(Index::Strategy::pass);
    w.octave.set_values(12.0);

    SECTION("position 0.0") {
        cursor.set_values(0.0);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{0, 2}));
    }

    SECTION("Test get with position 1.0") {
        cursor.set_values(1.0);
        REQUIRE_THAT(runner.step(), m11::emptyf());
    }

    SECTION("Test get with position 0.5") {
        cursor.set_values(0.5);
        REQUIRE_THAT(runner.step(), m11::eqf(5));
    }

    SECTION("Test get with position 2.5") {
        cursor.set_values(2.5);
        REQUIRE_THAT(runner.step(), m11::emptyf());
    }

    SECTION("Test get with position -1.5") {
        cursor.set_values(-1.5);
        REQUIRE_THAT(runner.step(), m11::emptyf());
    }

    SECTION("Test get with empty mapping") {
        cursor.set_values(Voices<double>::empty_like());
        REQUIRE_THAT(runner.step(), m11::emptyf());
    }

    SECTION("Test get with mixed mappings") {
        Voices<int> mixed_corpus{{0, 2}, {}, {8, 10}};
        w.corpus.set_values(mixed_corpus);

        cursor.set_values(0.0);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{0, 2}));
        cursor.set_values(0.4);
        REQUIRE_THAT(runner.step(), m11::emptyf());
        cursor.set_values(0.8);
        REQUIRE_THAT(runner.step(), m1m::eqf(Voice<double>{8, 10}));
    }
}

// ==============================================================================================


TEST_CASE("Interpolator: No rounding errors for corpora with sizes between 1 and 1000", "[interpolator]") {
    InterpolatorIntWrapper<> w;
    NodeRunner runner{&w.interpolator};

    auto& cursor = w.cursor;

    auto mode = GENERATE(
        Index::Strategy::cont
        // Index::Strategy::mod,
        // Index::Strategy::clip,
        // Index::Strategy::pass
    );

    w.mode.set_values(mode);

    auto corpus_size = GENERATE(
        1, 2, 3, 4, 5, 6, 7, 8, 9,
        10, 20, 30, 50, 60, 70, 80, 90,
        100, 200, 300, 400, 500, 600, 700, 800, 900,
        990, 991, 992, 993, 994, 995, 996, 997, 998, 999, 1000);

    CAPTURE(corpus_size);

    auto corpus = Voices<int>::transposed(Vec<int>::range(corpus_size));
    w.corpus.set_values(corpus);

    // Test rounding for every position from 0.0 to 1.0
    for (int cursor_index = 0; cursor_index < corpus_size; ++cursor_index) {
        double position = static_cast<double>(cursor_index) / static_cast<double>(corpus_size);

        cursor.set_values(position);
        REQUIRE_THAT(runner.step(), m11::eqf(static_cast<double>(cursor_index)));
    }
}