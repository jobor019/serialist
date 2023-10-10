#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/algo/collections/voices.h"

TEST_CASE("Voices constructor with a vector of Voice objects") {
    // Create a vector of Voice objects
    Vec<Voice<int>> v;
    v.append(Voice<int>({1, 2, 3}));
    v.append({4, 5, 6});
    v.append({7, 8, 9});

    Voices<int> voices(std::move(v));

    REQUIRE(voices.size() == 3);
    REQUIRE(voices.first().value() == 1);
    REQUIRE(voices.first_or(100) == 1);
}


TEST_CASE("Voices constructor with a number of voices") {
    auto voices = Voices<int>::zeros(4);

    REQUIRE(voices.size() == 4);
    REQUIRE(voices.is_empty_like() == true);
}
//
TEST_CASE("Voices::create_empty_like") {
    Voices<int> emptyVoices = Voices<int>::empty_like();

    REQUIRE(emptyVoices.size() == 1);
    REQUIRE(emptyVoices.is_empty_like() == true);
}


TEST_CASE("Voices::zeros") {
    SECTION("Create Voices with zero values") {
        auto voices = Voices<int>::zeros(4);

        REQUIRE(voices.size() == 4);
        REQUIRE(voices.is_empty_like() == true);
    }

    SECTION("Create Voices with zero values and specify the number of voices") {
        auto voices = Voices<int>::zeros(6);

        REQUIRE(voices.size() == 6);
        REQUIRE(voices.is_empty_like() == true);
    }
}

TEST_CASE("Voices::singular") {
    SECTION("Create a single Voice with a specific value") {
        auto voices = Voices<int>::singular(42);

        REQUIRE(voices.size() == 1);
        REQUIRE(voices.first().value() == 42);
    }

    SECTION("Create multiple Voices with the same value") {
        auto voices = Voices<int>::singular(99, 5);

        REQUIRE(voices.size() == 5);
        REQUIRE(voices[0].first().has_value());
        REQUIRE(voices[0].first().value() == 99);
        REQUIRE_FALSE(voices[1].first().has_value());
        REQUIRE_FALSE(voices[2].first().has_value());
        REQUIRE_FALSE(voices[3].first().has_value());
        REQUIRE_FALSE(voices[4].first().has_value());
    }
}

TEST_CASE("Voices::repeated") {
    SECTION("Create multiple Voices with repeated values") {
        auto voices = Voices<int>::repeated(7, 3);

        REQUIRE(voices.size() == 3);
        for (const auto& voice : voices) {
            REQUIRE(voice.first().value() == 7);
        }
    }
}



TEST_CASE("Voices::transposed") {
    auto voice1 = Voice<int>{1, 2, 3};
    Voices<int> voices = Voices<int>::transposed(voice1);

    REQUIRE(voices.size() == 3);
    REQUIRE(voices.first().value() == 1);
}


TEST_CASE("Voices::operator==") {
    auto voice1 = Voice<int>{1, 2, 3};
    auto voice2 = Voice<int>{4, 5, 6};
    Voices<int> voices1({voice1, voice2});
    Voices<int> voices2({voice1, voice2});

    REQUIRE(voices1 == voices2);

    auto voice3 = Voice<int>{7, 8, 9};
    Voices<int> voices3({voice1, voice3});

    REQUIRE_FALSE(voices1 == voices3);
}


TEST_CASE("Voices::cloned") {
    auto voice1 = Voice<int>{1, 2, 3};
    Voices<int> voices1({voice1});

    Voices<int> voices2 = voices1.cloned();

    REQUIRE(voices1 == voices2);
}


TEST_CASE("Voices::clear") {
    auto voice1 = Voice<int>{1, 2, 3};
    Voices<int> voices1({voice1});

    voices1.clear();

    REQUIRE(voices1.size() == 1);
    REQUIRE(voices1.is_empty_like() == true);

    voices1.clear(3);

    REQUIRE(voices1.size() == 3);
    REQUIRE(voices1.is_empty_like() == true);
}


TEST_CASE("Voices::size") {
    auto voice1 = Voice<int>{1, 2, 3};
    Voices<int> voices1({voice1});
    auto voices2 = Voices<int>::zeros(5);

    REQUIRE(voices1.size() == 1);
    REQUIRE(voices2.size() == 5);
}


TEST_CASE("Voices::merge") {
    auto voice1 = Voice<int>{1, 2, 3};
    auto voice2 = Voice<int>{4, 5, 6};
    Voices<int> voices1({voice1});
    Voices<int> voices2({voice2});

    voices1.merge(voices2);

    REQUIRE(voices1.size() == 1);
    REQUIRE(voices1.first().value() == 1);
    REQUIRE(voices1.vec()[0] == Voice<int>({1, 2, 3, 4, 5, 6}));
}


TEST_CASE("Voices::merge_uneven") {
    auto voice1 = Voice<int>{1, 2, 3};
    auto voice2 = Voice<int>{4, 5, 6, 7};
    Voices<int> voices1({voice1});
    Voices<int> voices2({voice2});

    voices1.merge_uneven(voices2, false);

    REQUIRE(voices1.size() == 1);
    REQUIRE(voices1.vec()[0] == Voice<int>({1, 2, 3, 4, 5, 6, 7}));

    Voices<int> voices3({voice1});
    Voices<int> voices4({voice2});

    voices3.merge_uneven(voices4, true);

    REQUIRE(voices3.size() == 1);
    REQUIRE(voices3.vec()[0] == Voice<int>({1, 2, 3, 4, 5, 6, 7}));
}


TEST_CASE("Voices::is_empty_like") {
    auto voice1 = Voice<int>{};
    Voices<int> voices1({voice1});

    REQUIRE(voices1.is_empty_like() == true);

    auto voice2 = Voice<int>::singular(1);
    Voices<int> voices2({voice2});

    REQUIRE(voices2.is_empty_like() == false);
}


TEST_CASE("Voices::first") {
    auto voice1 = Voice<int>{1, 2, 3};
    Voices<int> voices1({voice1});

    REQUIRE(voices1.first().value() == 1);

     auto emptyVoices = Voices<int>::empty_like();

    REQUIRE_FALSE(emptyVoices.first().has_value());
}


TEST_CASE("Voices::first_or") {
    auto voice1 = Voice<int>{1, 2, 3};
    Voices<int> voices1({voice1});

    REQUIRE(voices1.first_or(100) == 1);

    auto emptyVoices = Voices<int>::empty_like();

    REQUIRE(emptyVoices.first_or(100) == 100);
}


TEST_CASE("Voices::fronts") {
    auto voice1 = Voice<int>{1, 2, 3};
    auto voice2 = Voice<int>{4, 5, 6};
    Voices<int> voices1({voice1, voice2});

    Vec<std::optional<int>> fronts = voices1.fronts();

    REQUIRE(fronts.size() == 2);
    REQUIRE(fronts[0].value() == 1);
    REQUIRE(fronts[1].value() == 4);

    auto emptyVoices = Voices<int>::zeros(2);

    fronts = emptyVoices.fronts();

    REQUIRE(fronts.size() == 2);
    REQUIRE_FALSE(fronts[0].has_value());
    REQUIRE_FALSE(fronts[1].has_value());
}


TEST_CASE("Voices::fronts_or") {
    auto voice1 = Voice<int>{1, 2, 3};
    auto voice2 = Voice<int>{4, 5, 6};
    Voices<int> voices1({voice1, voice2});

    Vec<int> fronts = voices1.fronts_or(100);

    REQUIRE(fronts.size() == 2);
    REQUIRE(fronts[0] == 1);
    REQUIRE(fronts[1] == 4);

    auto emptyVoices = Voices<int>::zeros(2);

    fronts = emptyVoices.fronts_or(100);

    REQUIRE(fronts.size() == 2);
    REQUIRE(fronts[0] == 100);
    REQUIRE(fronts[1] == 100);
}


TEST_CASE("Voices::adapted_to") {
    auto voice1 = Voice<int>{1, 2, 3};
    Voices<int> voices1({voice1});

    Voices<int> voices2 = voices1.adapted_to(3);
    Voices<int> voices3 = voices1.adapted_to(2);

    REQUIRE(voices2.size() == 3);
    REQUIRE(voices3.size() == 2);
}


TEST_CASE("Voices::as_type") {
    auto voice1 = Voice<int>{1, 2, 3};
    Voices<int> voices1({voice1});

    Voices<double> voices2 = voices1.as_type<double>();
}


TEST_CASE("Voices::vec and Voices::vec_mut") {
    auto voice1 = Voice<int>{1, 2, 3};
    Voices<int> voices1({voice1});

    const Vec<Voice<int>>& constVec = voices1.vec();
    Vec<Voice<int>>& mutableVec = voices1.vec_mut();

    REQUIRE(constVec.size() == 1);
    REQUIRE(mutableVec.size() == 1);

    mutableVec[0] = {4, 5, 6};

    REQUIRE(constVec[0].size() == 3);
    REQUIRE(constVec[0][0] == 4);
}


TEST_CASE("Operator[] for Mutable Voices") {
    Voices<int> voices({{1, 2, 3}, {4, 5}, {6}});
    REQUIRE(voices.size() == 3);

    SECTION("Accessing valid indices") {
        REQUIRE(voices[0] == Voice<int>({1, 2, 3}));
        REQUIRE(voices[1] == Voice<int>({4, 5}));
        REQUIRE(voices[2] == Voice<int>({6}));
    }

    SECTION("Accessing out-of-range index") {
        REQUIRE_THROWS_AS(voices[3], std::out_of_range);
    }

    SECTION("Modifying values using operator[]") {
        voices[0][0] = 100;
        REQUIRE(voices[0] == Voice<int>({100, 2, 3}));
        voices[2].vector_mut().push_back(7);
        REQUIRE(voices[2] == Voice<int>({6, 7}));
    }
}

TEST_CASE("Operator[] for Const Voices") {
    const Voices<double> voices({{1.1, 2.2}, {3.3}});
    REQUIRE(voices.size() == 2);

    SECTION("Accessing valid indices") {
        REQUIRE(voices[0] == Voice<double>({1.1, 2.2}));
        REQUIRE(voices[1] == Voice<double>({3.3}));
    }

    SECTION("Accessing out-of-range index") {
        REQUIRE_THROWS_AS(voices[2], std::out_of_range);
    }
}





