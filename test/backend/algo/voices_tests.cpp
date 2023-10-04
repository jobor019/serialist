#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/algo/voice/voices.h"

TEST_CASE("Voices constructor with a vector of Voice objects") {
    // Create a vector of Voice objects
    Vec<Voice<int>> v;
    v.append(Voice<int>({1, 2, 3}));
    v.append({4, 5, 6});
    v.append({7, 8, 9});

    Voices<int> voices(std::move(v));

    REQUIRE(voices.size() == 3);
    REQUIRE(voices.front().value() == 1);
    REQUIRE(voices.front_or(100) == 1);
}


TEST_CASE("Voices constructor with a number of voices") {
    Voices<int> voices(4);

    REQUIRE(voices.size() == 4);
    REQUIRE(voices.is_empty_like() == true);
}
//
TEST_CASE("Voices::create_empty_like") {
    Voices<int> emptyVoices = Voices<int>::create_empty_like();

    REQUIRE(emptyVoices.size() == 1);
    REQUIRE(emptyVoices.is_empty_like() == true);
}


TEST_CASE("Voices::transposed") {
    Voice<int> voice1 = {1, 2, 3};
    Voices<int> voices = Voices<int>::transposed(voice1);

    REQUIRE(voices.size() == 3);
    REQUIRE(voices.front().value() == 1);
}


TEST_CASE("Voices::operator==") {
    Voice<int> voice1 = {1, 2, 3};
    Voice<int> voice2 = {4, 5, 6};
    Voices<int> voices1({voice1, voice2});
    Voices<int> voices2({voice1, voice2});

    REQUIRE(voices1 == voices2);

    Voice<int> voice3 = {7, 8, 9};
    Voices<int> voices3({voice1, voice3});

    REQUIRE_FALSE(voices1 == voices3);
}


TEST_CASE("Voices::cloned") {
    Voice<int> voice1 = {1, 2, 3};
    Voices<int> voices1({voice1});

    Voices<int> voices2 = voices1.cloned();

    REQUIRE(voices1 == voices2);
}


TEST_CASE("Voices::clear") {
    Voice<int> voice1 = {1, 2, 3};
    Voices<int> voices1({voice1});

    voices1.clear();

    REQUIRE(voices1.size() == 1);
    REQUIRE(voices1.is_empty_like() == true);

    voices1.clear(3);

    REQUIRE(voices1.size() == 3);
    REQUIRE(voices1.is_empty_like() == true);
}


TEST_CASE("Voices::size") {
    Voice<int> voice1 = {1, 2, 3};
    Voices<int> voices1({voice1});
    Voices<int> voices2(5);

    REQUIRE(voices1.size() == 1);
    REQUIRE(voices2.size() == 5);
}


TEST_CASE("Voices::merge") {
    Voice<int> voice1 = {1, 2, 3};
    Voice<int> voice2 = {4, 5, 6};
    Voices<int> voices1({voice1});
    Voices<int> voices2({voice2});

    voices1.merge(voices2);

    REQUIRE(voices1.size() == 1);
    REQUIRE(voices1.front().value() == 1);
    REQUIRE(voices1.vec()[0] == Voice<int>({1, 2, 3, 4, 5, 6}));
}


TEST_CASE("Voices::merge_uneven") {
    Voice<int> voice1 = {1, 2, 3};
    Voice<int> voice2 = {4, 5, 6, 7};
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
    Voice<int> voice1 = {};
    Voices<int> voices1({voice1});

    REQUIRE(voices1.is_empty_like() == true);

    Voice<int> voice2 = {1};
    Voices<int> voices2({voice2});

    REQUIRE(voices2.is_empty_like() == false);
}


TEST_CASE("Voices::front") {
    Voice<int> voice1 = {1, 2, 3};
    Voices<int> voices1({voice1});

    REQUIRE(voices1.front().value() == 1);

    Voices<int> emptyVoices(1);

    REQUIRE_FALSE(emptyVoices.front().has_value());
}


TEST_CASE("Voices::front_or") {
    Voice<int> voice1 = {1, 2, 3};
    Voices<int> voices1({voice1});

    REQUIRE(voices1.front_or(100) == 1);

    Voices<int> emptyVoices(1);

    REQUIRE(emptyVoices.front_or(100) == 100);
}


TEST_CASE("Voices::fronts") {
    Voice<int> voice1 = {1, 2, 3};
    Voice<int> voice2 = {4, 5, 6};
    Voices<int> voices1({voice1, voice2});

    Vec<std::optional<int>> fronts = voices1.fronts();

    REQUIRE(fronts.size() == 2);
    REQUIRE(fronts[0].value() == 1);
    REQUIRE(fronts[1].value() == 4);

    Voices<int> emptyVoices(2);

    fronts = emptyVoices.fronts();

    REQUIRE(fronts.size() == 2);
    REQUIRE_FALSE(fronts[0].has_value());
    REQUIRE_FALSE(fronts[1].has_value());
}


TEST_CASE("Voices::fronts_or") {
    Voice<int> voice1 = {1, 2, 3};
    Voice<int> voice2 = {4, 5, 6};
    Voices<int> voices1({voice1, voice2});

    Vec<int> fronts = voices1.fronts_or(100);

    REQUIRE(fronts.size() == 2);
    REQUIRE(fronts[0] == 1);
    REQUIRE(fronts[1] == 4);

    Voices<int> emptyVoices(2);

    fronts = emptyVoices.fronts_or(100);

    REQUIRE(fronts.size() == 2);
    REQUIRE(fronts[0] == 100);
    REQUIRE(fronts[1] == 100);
}


TEST_CASE("Voices::adapted_to") {
    Voice<int> voice1 = {1, 2, 3};
    Voices<int> voices1({voice1});

    Voices<int> voices2 = voices1.adapted_to(3);
    Voices<int> voices3 = voices1.adapted_to(2);

    REQUIRE(voices2.size() == 3);
    REQUIRE(voices3.size() == 2);
}


TEST_CASE("Voices::as_type") {
    Voice<int> voice1 = {1, 2, 3};
    Voices<int> voices1({voice1});

    Voices<double> voices2 = voices1.as_type<double>();
}


TEST_CASE("Voices::vec and Voices::vec_mut") {
    Voice<int> voice1 = {1, 2, 3};
    Voices<int> voices1({voice1});

    const Vec<Voice<int>>& constVec = voices1.vec();
    Vec<Voice<int>>& mutableVec = voices1.vec_mut();

    REQUIRE(constVec.size() == 1);
    REQUIRE(mutableVec.size() == 1);

    mutableVec[0] = {4, 5, 6};

    REQUIRE(constVec[0].size() == 3);
    REQUIRE(constVec[0][0] == 4);
}



