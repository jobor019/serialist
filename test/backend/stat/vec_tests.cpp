
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/stat/vec.h"

TEST_CASE("Vec Constructors and Basic Operations") {
    SECTION("Default Constructor and Size") {
        Vec<int> empty_vector{};
        REQUIRE(empty_vector.size() == 0);

        Vec v1{1, 2, 3};
        REQUIRE(v1.size() == 3);
    }

    SECTION("Arithmetic Operations") {
        Vec vector1({1, 2, 3});
        Vec vector2({4, 5, 6});

        Vec sum = vector1 + vector2;
        REQUIRE(sum.size() == vector1.size());
        REQUIRE(sum[0] == 5);
        REQUIRE(sum[1] == 7);
        REQUIRE(sum[2] == 9);

        Vec<int> difference = vector2 - vector1;
        REQUIRE(difference.size() == vector1.size());
        REQUIRE(difference[0] == 3);
        REQUIRE(difference[1] == 3);
        REQUIRE(difference[2] == 3);

        Vec<int> product = vector1 * vector2;
        REQUIRE(product.size() == vector1.size());
        REQUIRE(product[0] == 4);
        REQUIRE(product[1] == 10);
        REQUIRE(product[2] == 18);

        Vec<int> quotient = vector2 / vector1;
        REQUIRE(quotient.size() == vector1.size());
        REQUIRE(quotient[0] == 4);
        REQUIRE(quotient[1] == 2);
        REQUIRE(quotient[2] == 2);
    }
}

TEST_CASE("Histogram") {
    Vec vector{0, 1, 2, 2, 3, 3, 3, 4, 4, 5, 5, 5};

    SECTION("Histogram Test") {
        Vec<int> histogram = vector.histogram<int>(6);
        REQUIRE(histogram.size() == 6);
        REQUIRE(histogram[0] == 1);
        REQUIRE(histogram[1] == 1);
        REQUIRE(histogram[2] == 2);
        REQUIRE(histogram[3] == 3);
        REQUIRE(histogram[4] == 2);
        REQUIRE(histogram[5] == 3);
    }
}

TEST_CASE("Element-wise Operations") {
    Vec vector1({1, 2, 3});
    Vec vector2({4, 5, 6});

    SECTION("Element-wise Addition") {
        Vec<int> added = vector1 + vector2;
        REQUIRE(added.size() == vector1.size());
        REQUIRE(added[0] == 5);
        REQUIRE(added[1] == 7);
        REQUIRE(added[2] == 9);
    }

    SECTION("Element-wise Subtraction") {
        Vec<int> subtracted = vector2 - vector1;
        REQUIRE(subtracted.size() == vector1.size());
        REQUIRE(subtracted[0] == 3);
        REQUIRE(subtracted[1] == 3);
        REQUIRE(subtracted[2] == 3);
    }

    SECTION("Element-wise Multiplication") {
        Vec<int> multiplied = vector1 * vector2;
        REQUIRE(multiplied.size() == vector1.size());
        REQUIRE(multiplied[0] == 4);
        REQUIRE(multiplied[1] == 10);
        REQUIRE(multiplied[2] == 18);
    }

    SECTION("Element-wise Division") {
        Vec<int> divided = vector2 / vector1;
        REQUIRE(divided.size() == vector1.size());
        REQUIRE(divided[0] == 4);
        REQUIRE(divided[1] == 2);
        REQUIRE(divided[2] == 2);
    }
}

TEST_CASE("Other Vec Functions") {
    Vec vector({1, 2, 3, 4, 5});

    SECTION("Create Empty Vector Like") {
        Vec empty_vector = Vec<int>::create_empty_like(vector.size());
        REQUIRE(empty_vector.size() == vector.size());
        for (std::size_t i = 0; i < vector.size(); i++) {
            REQUIRE(empty_vector[i] == 0);
        }
    }

    SECTION("Apply Function") {
        Vec applied_vector(vector);
        applied_vector.apply([](int a, int b) { return a * b; }, 2);
        REQUIRE(applied_vector[0] == 2);
        REQUIRE(applied_vector[1] == 4);
        REQUIRE(applied_vector[2] == 6);
        REQUIRE(applied_vector[3] == 8);
        REQUIRE(applied_vector[4] == 10);
    }

}

TEST_CASE("Other Vec Functions (Continued)") {
    Vec vector({1, 2, 3, 4, 5});

    SECTION("Slice") {
        Vec<int> sliced = vector.slice(1, 4);
        REQUIRE(sliced.size() == 3);
        REQUIRE(sliced[0] == 2);
        REQUIRE(sliced[1] == 3);
        REQUIRE(sliced[2] == 4);
    }


    SECTION("Sum") {
        int sum = vector.sum();
        REQUIRE(sum == 15);
    }

    SECTION("Cumulative Sum (cumsum)") {
        int cumulativeSum = vector.cumsum();
        REQUIRE(cumulativeSum == 15);
    }

    SECTION("Mean") {
        REQUIRE_THAT(vector.mean(), Catch::Matchers::WithinRel(3.0, 0.001));
    }

    SECTION("Power (pow)") {
        Vec<int> powered = vector.pow(2);
        REQUIRE(powered.size() == vector.size());
        REQUIRE(powered[0] == 1);
        REQUIRE(powered[1] == 4);
        REQUIRE(powered[2] == 9);
        REQUIRE(powered[3] == 16);
        REQUIRE(powered[4] == 25);
    }

    SECTION("As Type Conversion (as_type)") {
        Vec<double> doubleVector = vector.as_type<double>();
        REQUIRE(doubleVector.size() == vector.size());
        REQUIRE(doubleVector[0] == 1.0);
        REQUIRE(doubleVector[1] == 2.0);
        REQUIRE(doubleVector[2] == 3.0);
        REQUIRE(doubleVector[3] == 4.0);
        REQUIRE(doubleVector[4] == 5.0);
    }

    SECTION("Normalize") {
        Vec<double> normalized_vector(vector.as_type<double>());
        normalized_vector.normalize();
        REQUIRE_THAT(normalized_vector.max(), Catch::Matchers::WithinRel(1.0, 0.001));
    }

    SECTION("Sort Ascending") {
        Vec<int> sorted = vector.sort(true);
        REQUIRE(sorted.size() == vector.size());
        REQUIRE(sorted[0] == 1);
        REQUIRE(sorted[1] == 2);
        REQUIRE(sorted[2] == 3);
        REQUIRE(sorted[3] == 4);
        REQUIRE(sorted[4] == 5);
    }

    SECTION("Sort Descending") {
        Vec<int> sorted = vector.sort(false);
        REQUIRE(sorted.size() == vector.size());
        REQUIRE(sorted[0] == 5);
        REQUIRE(sorted[1] == 4);
        REQUIRE(sorted[2] == 3);
        REQUIRE(sorted[3] == 2);
        REQUIRE(sorted[4] == 1);
    }

    SECTION("Max") {
        int maxValue = vector.max();
        REQUIRE(maxValue == 5);
    }

    SECTION("Min") {
        int minValue = vector.min();
        REQUIRE(minValue == 1);
    }
}

TEST_CASE("Subscripting and Edge Cases") {
    Vec vector({1, 2, 3, 4, 5});

    SECTION("Subscripting Operator ([])") {
        REQUIRE(vector[0] == 1);
        REQUIRE(vector[1] == 2);
        REQUIRE(vector[2] == 3);
        REQUIRE(vector[3] == 4);
        REQUIRE(vector[4] == 5);
    }

    SECTION("Subscripting Operator ([]) with Negative Indices") {
        REQUIRE(vector[-1] == 5);
        REQUIRE(vector[-2] == 4);
        REQUIRE(vector[-3] == 3);
        REQUIRE(vector[-4] == 2);
        REQUIRE(vector[-5] == 1);
    }

    SECTION("Subscripting Operator ([]) Out of Range") {
        // Test with an out-of-range index
        REQUIRE_THROWS(vector[6]);
        REQUIRE_THROWS(vector[-6]);
    }

    SECTION("Apply Function with Binary Mask") {
        Vec valuesToApply({10, 30, 50});
        Vec binaryMask({true, false, true, false, true});

        Vec<int> result = vector;
        result.apply([](int a, int b) { return a + b; }, valuesToApply, binaryMask);

        REQUIRE(result[0] == 11);
        REQUIRE(result[1] == 2);  // No change, binaryMask is false
        REQUIRE(result[2] == 33);
        REQUIRE(result[3] == 4);  // No change, binaryMask is false
        REQUIRE(result[4] == 55);
    }

    SECTION("Apply Function with Binary Mask - Size Mismatch") {
        Vec valuesToApply({10, 20, 30});
        Vec binaryMask({true, false});

        REQUIRE_THROWS(vector.apply([](int a, int b) { return a + b; }, valuesToApply, binaryMask));
    }

    SECTION("Apply Function with Indices") {
        Vec valuesToApply({10, 20, 30});
        Vec<long> indicesToApply({1, 3, 4});

        Vec<int> result = vector;
        result.apply([](int a, int b) { return a + b; }, valuesToApply, indicesToApply);

        REQUIRE(result[0] == 1);
        REQUIRE(result[1] == 12);
        REQUIRE(result[2] == 3);
        REQUIRE(result[3] == 24);
        REQUIRE(result[4] == 35);
    }

    SECTION("Apply Function with Indices - Size Mismatch") {
        Vec valuesToApply({10, 20, 30});
        Vec<long> indicesToApply({1, 3});

        REQUIRE_THROWS(vector.apply([](int a, int b) { return a + b; }, valuesToApply, indicesToApply));
    }
}


