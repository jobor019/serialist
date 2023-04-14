#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "../src/mapping.h"
#include "../src/accessor.h"


TEST_CASE("Test ContinueInterpolator with MapElement<int>") {
    Mapping<int> mapping{{  0, 2}
                         , {4}
                         , {5}
                         , {7, 9, 11}};
    ContinueInterpolator<int> interpolator(12);

    SECTION("Test get with position 0.0") {
        auto result = interpolator.get(0.0, mapping);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 2);
    }

    SECTION("Test get with position 1.0") {
        auto result = interpolator.get(1.0, mapping);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 12);
        REQUIRE(result[1] == 14);
    }

    SECTION("Test get with position 0.5") {
        auto result = interpolator.get(0.5, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with position 2.5") {
        auto result = interpolator.get(2.5, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5 + 12 * 2);
    }

    SECTION("Test get with position -1.5") {
        auto result = interpolator.get(-1.5, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5 - 12 * 2);
    }

    SECTION("Test get with empty mapping") {
        Mapping<int> empty_mapping{};
        auto result = interpolator.get(2.25, empty_mapping);
        REQUIRE(result.empty());
    }
}


// ==============================================================================================

TEST_CASE("Test ModuloInterpolator with MapElement<int>") {
    Mapping<int> mapping{{  0, 2}
                         , {4}
                         , {5}
                         , {7, 9, 11}};
    ModuloInterpolator<int> interpolator;

    SECTION("Test get with position 0.0") {
        auto result = interpolator.get(0.0, mapping);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 2);
    }

    SECTION("Test get with position 1.0") {
        auto result = interpolator.get(1.0, mapping);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 2);
    }

    SECTION("Test get with position 0.5") {
        auto result = interpolator.get(0.5, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with position 2.5") {
        auto result = interpolator.get(2.5, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with position -1.5") {
        auto result = interpolator.get(-1.5, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with empty mapping") {
        Mapping<int> empty_mapping{};
        auto result = interpolator.get(2.25, empty_mapping);
        REQUIRE(result.empty());
    }
}


// ==============================================================================================

TEST_CASE("Test ClipInterpolator with MapElement<int>") {
    Mapping<int> mapping{{  0, 2}
                         , {4}
                         , {5}
                         , {7, 9, 11}};
    ClipInterpolator<int> interpolator;

    SECTION("Test get with position 0.0") {
        auto result = interpolator.get(0.0, mapping);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 2);
    }

    SECTION("Test get with position 1.0") {
        auto result = interpolator.get(1.0, mapping);
        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == 7);
        REQUIRE(result[1] == 9);
        REQUIRE(result[2] == 11);
    }

    SECTION("Test get with position 0.5") {
        auto result = interpolator.get(0.5, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with position 2.5") {
        auto result = interpolator.get(2.5, mapping);
        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == 7);
        REQUIRE(result[1] == 9);
        REQUIRE(result[2] == 11);
    }

    SECTION("Test get with position -1.5") {
        auto result = interpolator.get(-1.5, mapping);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 2);
    }

    SECTION("Test get with empty mapping") {
        Mapping<int> empty_mapping{};
        auto result = interpolator.get(2.25, empty_mapping);
        REQUIRE(result.empty());
    }
}


// ==============================================================================================

TEST_CASE("Test PassInterpolator with MapElement<int>") {
    Mapping<int> mapping{{  0, 2}
                         , {4}
                         , {5}
                         , {7, 9, 11}};
    PassInterpolator<int> interpolator;

    SECTION("Test get with position 0.0") {
        auto result = interpolator.get(0.0, mapping);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 2);
    }

    SECTION("Test get with position 1.0") {
        auto result = interpolator.get(1.0, mapping);
        REQUIRE(result.empty());
    }

    SECTION("Test get with position 0.5") {
        auto result = interpolator.get(0.5, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with position 2.5") {
        auto result = interpolator.get(2.5, mapping);
        REQUIRE(result.empty());
    }

    SECTION("Test get with position -1.5") {
        auto result = interpolator.get(-1.5, mapping);
        REQUIRE(result.empty());
    }

    SECTION("Test get with empty mapping") {
        Mapping<int> empty_mapping{};
        auto result = interpolator.get(2.25, empty_mapping);
        REQUIRE(result.empty());
    }
}

// ==============================================================================================

TEST_CASE("Test IdentityAccessor with MapElement<int>") {
    Mapping<int> mapping{{  0, 2}
                         , {4}
                         , {5}
                         , {7, 9, 11}};
    IdentityAccessor<int> accessor;

    SECTION("Test get with position 0") {
        auto result = accessor.get(0, mapping);
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0);
        REQUIRE(result[1] == 2);
    }

    SECTION("Test get with position 1") {
        auto result = accessor.get(1, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 4);
    }

    SECTION("Test get with position 2") {
        auto result = accessor.get(2, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with position 3") {
        auto result = accessor.get(3, mapping);
        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == 7);
        REQUIRE(result[1] == 9);
        REQUIRE(result[2] == 11);
    }

    SECTION("Test get with position out of bounds") {
        auto result = accessor.get(10, mapping);
        REQUIRE(result.empty());
    }

    SECTION("Test get with position -1") {
        auto result = accessor.get(-1, mapping);
        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == 7);
        REQUIRE(result[1] == 9);
        REQUIRE(result[2] == 11);
    }

    SECTION("Test get with empty mapping") {
        Mapping<int> empty_mapping{};
        auto result = accessor.get(2, empty_mapping);
        REQUIRE(result.empty());
    }
}

// ==============================================================================================

TEST_CASE("Test NthAccessor with MapElement<int>") {
    Mapping<int> mapping{{  0, 2}
                         , {4}
                         , {5}
                         , {7, 9, 11}};


    SECTION("Test get nth=2 with position 0") {
        NthAccessor<int> accessor(2);
        auto result = accessor.get(0, mapping);
        REQUIRE(result.empty());
    }

    SECTION("Test get nth=2 with position 1") {
        NthAccessor<int> accessor(2);
        auto result = accessor.get(1, mapping);
        REQUIRE(result.empty());
    }

    SECTION("Test get nth=2 with position 2") {
        NthAccessor<int> accessor(2);
        auto result = accessor.get(2, mapping);
        REQUIRE(result.empty());
    }

    SECTION("Test get nth=2 with position 3") {
        NthAccessor<int> accessor(2);
        auto result = accessor.get(3, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 11);
    }

    SECTION("Test get nth=-1 with position 0") {
        NthAccessor<int> accessor(-1);
        auto result = accessor.get(0, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 2);
    }

    SECTION("Test get nth=-1 with position 1") {
        NthAccessor<int> accessor(-1);
        auto result = accessor.get(1, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 4);
    }

    SECTION("Test get nth=-1 with position 2") {
        NthAccessor<int> accessor(-1);
        auto result = accessor.get(2, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get nth=-1 with position 3") {
        NthAccessor<int> accessor(-1);
        auto result = accessor.get(3, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 11);
    }

    SECTION("Test get with position out of bounds") {
        NthAccessor<int> accessor(2);
        auto result = accessor.get(10, mapping);
        REQUIRE(result.empty());
    }

    SECTION("Test get with position -1") {
        NthAccessor<int> accessor(2);
        auto result = accessor.get(-1, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 11);
    }

    SECTION("Test get with empty mapping") {
        NthAccessor<int> accessor(2);
        Mapping<int> empty_mapping{};
        auto result = accessor.get(2, empty_mapping);
        REQUIRE(result.empty());
    }
}


// ==============================================================================================

TEST_CASE("Test FirstAccessor with MapElement<int>") {
    Mapping<int> mapping{{  0, 2}
                         , {4}
                         , {5}
                         , {7, 9, 11}};
    FirstAccessor<int> accessor;

    SECTION("Test get with position 0") {
        auto result = accessor.get(0, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 0);
    }

    SECTION("Test get with position 1") {
        auto result = accessor.get(1, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 4);
    }

    SECTION("Test get with position 2") {
        auto result = accessor.get(2, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 5);
    }

    SECTION("Test get with position 3") {
        auto result = accessor.get(3, mapping);
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 7);
    }
}





