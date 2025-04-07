
#include <unordered_set>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/algo/random/random.h"
#include "core/collections/vec.h"

using namespace serialist;

TEST_CASE("Random::next() returns values in [0, 1]") {
    Random random(0);
    for (int i = 0; i < 1000; ++i) {
        double value = random.next();
        REQUIRE(value >= 0.0);
        REQUIRE(value < 1.0);
    }
}

TEST_CASE("Random::next(lower_bound, upper_bound) returns values in [lower_bound, upper_bound]") {
    Random random(0);
    double lower_bound = 2.0;
    double upper_bound = 4.0;
    for (int i = 0; i < 1000; ++i) {
        double value = random.next(lower_bound, upper_bound);
        REQUIRE(value >= lower_bound);
        REQUIRE(value <= upper_bound);
    }
}

TEST_CASE("Random::choice() returns values from the vector") {
    Random random(0);
    Vec values = {1, 2, 3, 4};
    std::unordered_set<int> seen_values;

    for (int i = 0; i < 1000; ++i) {
        int value = random.choice(values);
        seen_values.insert(value);
    }

    // Ensure that each value has been seen at least once
    for (int v : values) {
        REQUIRE(seen_values.count(v) > 0);
    }
}

TEST_CASE("Random::weighted_choice() returns valid indices for a non-empty vector") {
    Random random(0);
    Vec values = {0.1, 0.3, 0.2, 0.4};

    std::vector<std::size_t> seen_indices(values.size(), 0);

    for (int i = 0; i < 1000; ++i) {
        std::size_t index = random.weighted_choice(values);
        REQUIRE(index >= 0);
        REQUIRE(index < values.size());
        seen_indices[index]++;
    }

    // Ensure that each index has been seen at least once
    for (std::size_t i = 0; i < values.size(); ++i) {
        REQUIRE(seen_indices[i] > 0);
    }
}

TEST_CASE("Random::choice() throws for an empty vector") {
    Random random(0);
    Vec<int> values;

    REQUIRE_THROWS(random.choice(values));
}

TEST_CASE("Random::weighted_choice() throws for an empty vector") {
    Random random(0);
    Vec<double> values;

    REQUIRE_THROWS(random.weighted_choice(values));
}

TEST_CASE("Random::scramble", "[scramble]") {
    Random random(0);

    SECTION("scrambling an empty vector should result in an empty vector") {
        Vec<int> empty_vector;
        Vec<int> scrambled = random.scramble(empty_vector);
        REQUIRE(scrambled.empty());
    }

    SECTION("scrambling a vector of size 1 should result in the same vector") {
        Vec single_value_vector = {42};
        Vec<int> scrambled = random.scramble(single_value_vector);
        REQUIRE(scrambled.size() == 1);
        REQUIRE(scrambled[0] == 42);
    }

    SECTION("scrambling a vector should produce a different order") {
        Vec original_vector = {1, 2, 3, 4, 5};
        Vec<int> scrambled = random.scramble(original_vector);

        REQUIRE(scrambled.size() == original_vector.size());

        bool elements_match = true;
        for (std::size_t i = 0; i < original_vector.size(); ++i) {
            if (original_vector[i] != scrambled[i]) {
                elements_match = false;
                break;
            }
        }
        REQUIRE_FALSE(elements_match);
    }
}


TEST_CASE("Random::choices() returns valid values without duplicates") {
    Random random(0);
    Vec values = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::unordered_set<int> seen_values;
    const int numIterations = 1000;

    for (int iteration = 0; iteration < numIterations; ++iteration) {
        Vec<int> chosen = random.choices(values, 5);

        REQUIRE(chosen.size() == 5);

        // Ensure that each chosen value is in the original vector
        for (int v : chosen) {
            REQUIRE(std::find(values.begin(), values.end(), v) != values.end());
            REQUIRE(seen_values.find(v) == seen_values.end()); // Check for duplicates
            seen_values.insert(v);
        }

        // Reset the set of seen values for the next iteration
        seen_values.clear();
    }
}


TEST_CASE("Random::choices() returns empty vector for num_choices = 0") {
    Random random(0);
    Vec values = {1, 2, 3, 4, 5};

    Vec<int> chosen = random.choices(values, 0);

    REQUIRE(chosen.empty());
}



