
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/algo/vec.h"

TEST_CASE("Vec Constructors", "[constructor]") {
    // Test default constructor
    Vec<int> v1;
    REQUIRE(v1.empty());

    // Test constructor with a vector of data
    Vec v2({1, 2, 3});
    REQUIRE(v2.size() == 3);
    REQUIRE(v2[0] == 1);
    REQUIRE(v2[1] == 2);
    REQUIRE(v2[2] == 3);

    // Test constructor with an initializer list
    Vec v3 = {4, 5, 6};
    REQUIRE(v3.size() == 3);
    REQUIRE(v3[0] == 4);
    REQUIRE(v3[1] == 5);
    REQUIRE(v3[2] == 6);
}


TEST_CASE("Vec operator==", "[operator]") {
    Vec v1({1, 2, 3});
    Vec v2({1, 2, 3});
    Vec v3({4, 5, 6});

    // Test equality between two equal vectors
    REQUIRE(v1 == v2);

    // Test inequality between two different vectors
    REQUIRE_FALSE(v1 == v3);
}


TEST_CASE("Vec operator+", "[operator]") {
    Vec v1({1, 2, 3});
    Vec v2({4, 5, 6});

    // Test addition operator
    Vec result = v1 + v2;
    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == 5);
    REQUIRE(result[1] == 7);
    REQUIRE(result[2] == 9);
}



TEST_CASE("Vec range", "[range]") {
    // Test creating a range of integers
    Vec<int> v1 = Vec<int>::range(1, 6, 2);
    REQUIRE(v1.size() == 3);
    REQUIRE(v1[0] == 1);
    REQUIRE(v1[1] == 3);
    REQUIRE(v1[2] == 5);

    // Test creating a range of doubles
    Vec<double> v2 = Vec<double>::range(1.0, 4.0, 0.5);
    REQUIRE(v2.size() == 7);
    REQUIRE(v2[0] == 1.0);
    REQUIRE(v2[1] == 1.5);
    REQUIRE(v2[6] == 4.0);
}

TEST_CASE("Vec repeated", "[repeated]") {
    // Test repeating a single value
    Vec<int> v1 = Vec<int>::repeated(3, 5);
    REQUIRE(v1.size() == 3);
    REQUIRE(v1[0] == 5);
    REQUIRE(v1[1] == 5);
    REQUIRE(v1[2] == 5);

    // Test repeating a vector of values
    Vec values = {1, 2, 3};
    auto v2 = Vec<int>::repeated(2, values);
    REQUIRE(v2.size() == 6);
    REQUIRE(v2[0] == 1);
    REQUIRE(v2[3] == 1);
    REQUIRE(v2[4] == 2);
    REQUIRE(v2[5] == 3);
}

TEST_CASE("Vec append", "[append]") {
    Vec<int> v;

    // Test appending elements
    v.append(1);
    REQUIRE(v.size() == 1);
    REQUIRE(v[0] == 1);

    v.append(2).append(3);
    REQUIRE(v.size() == 3);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);
    REQUIRE(v[2] == 3);
}

TEST_CASE("Vec concatenate", "[concatenate]") {
    Vec v1 = {1, 2};
    Vec v2 = {3, 4};

    // Test concatenating two vectors
    v1.concatenate(v2);
    REQUIRE(v1.size() == 4);
    REQUIRE(v1[0] == 1);
    REQUIRE(v1[1] == 2);
    REQUIRE(v1[2] == 3);
    REQUIRE(v1[3] == 4);
}

TEST_CASE("Vec remove", "[remove]") {
    Vec v = {1, 2, 3, 4, 5};

    // Test removing an element
    v.remove(3);
    REQUIRE(v.size() == 4);
    REQUIRE_FALSE(v.contains(3));

    // Test removing a non-existent element
    v.remove(6);
    REQUIRE(v.size() == 4);
}

TEST_CASE("Vec resize_append", "[resize_append]") {
    Vec v = {1, 2, 3};

    // Test resizing to a smaller size
    v.resize_append(2, 0);
    REQUIRE(v.size() == 2);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);

    // Test resizing to a larger size with append_value
    v.resize_append(5, 10);
    REQUIRE(v.size() == 5);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);
    REQUIRE(v[2] == 10);
    REQUIRE(v[3] == 10);
    REQUIRE(v[4] == 10);
}

TEST_CASE("Vec erase", "[erase]") {
    Vec v = {1, 2, 3, 4, 5};

    // Test erasing a single element by index
    v.erase(2); // Erase the element '3'
    REQUIRE(v.size() == 4);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);
    REQUIRE(v[2] == 4);
    REQUIRE(v[3] == 5);

    // Test erasing a range of elements by indices
    v.erase(1, 3); // Erase the elements '2' and '4'
    REQUIRE(v.size() == 2);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 5);
}


TEST_CASE("Vec slice", "[slice]") {
    Vec v = {1, 2, 3, 4, 5};

    // Test slicing from the beginning
    Vec<int> sliced = v.slice(0, 3);
    REQUIRE(sliced.size() == 3);
    REQUIRE(sliced[0] == 1);
    REQUIRE(sliced[1] == 2);
    REQUIRE(sliced[2] == 3);

    // Test slicing from a specific index to the end
    sliced = v.slice(2);
    REQUIRE(sliced.size() == 2);
    REQUIRE(sliced[0] == 3);
    REQUIRE(sliced[1] == 4);

    // Test slicing with negative indices
    sliced = v.slice(-3, -1);
    REQUIRE(sliced.size() == 2);
    REQUIRE(sliced[0] == 3);
    REQUIRE(sliced[1] == 4);
}

TEST_CASE("Vec as_type", "[as_type]") {
    Vec v = {1.5, 2.5, 3.5};

    // Test converting to an integer type
    Vec<int> converted = v.as_type<int>();
    REQUIRE(converted.size() == 3);
    REQUIRE(converted[0] == 1);
    REQUIRE(converted[1] == 2);
    REQUIRE(converted[2] == 3);
}

TEST_CASE("Vec apply", "[apply]") {
    Vec v = {1, 2, 3, 4, 5};

    // Test applying a function with a single value
    v.apply([](int x, int y) { return x + y; }, 10);
    REQUIRE(v.size() == 5);
    REQUIRE(v[0] == 11);
    REQUIRE(v[1] == 12);
    REQUIRE(v[2] == 13);
    REQUIRE(v[3] == 14);
    REQUIRE(v[4] == 15);

    // Test applying a function with another vector
    Vec other = {10, 20, 30, 40, 50};
    v.apply([](int x, int y) { return x - y; }, other);
    REQUIRE(v.size() == 5);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == -8);
    REQUIRE(v[2] == -17);
    REQUIRE(v[3] == -26);
    REQUIRE(v[4] == -35);
}


TEST_CASE("Vec size", "[size]") {
    Vec<int> v1;
    REQUIRE(v1.empty());

    Vec v2 = {1, 2, 3};
    REQUIRE(v2.size() == 3);
}

TEST_CASE("Vec contains", "[contains]") {
    Vec v = {1, 2, 3, 4, 5};

    // Test contains with an existing value
    REQUIRE(v.contains(3) == true);

    // Test contains with a non-existent value
    REQUIRE(v.contains(6) == false);
}

TEST_CASE("Vec empty", "[empty]") {
    Vec<int> v1;
    REQUIRE(v1.empty() == true);

    Vec v2 = {1, 2, 3};
    REQUIRE(v2.empty() == false);
}

TEST_CASE("Vec vector and vector_mut", "[vector]") {
    Vec v = {1, 2, 3, 4, 5};

    // Test accessing the underlying vector
    const std::vector<int>& constVector = v.vector();
    REQUIRE(constVector.size() == 5);
    REQUIRE(constVector[0] == 1);

    // Test modifying the underlying vector
    std::vector<int>& mutableVector = v.vector_mut();
    mutableVector.push_back(6);
    REQUIRE(v.size() == 6);
    REQUIRE(v[5] == 6);
}

TEST_CASE("Vec front", "[front]") {
    Vec v = {1, 2, 3, 4, 5};

    // Test front() when the vector is not empty
    std::optional<int> result1 = v.front();
    REQUIRE(result1.has_value() == true);
    REQUIRE(result1.value() == 1);

    // Test front() when the vector is empty
    Vec<int> emptyVec;
    std::optional<int> result2 = emptyVec.front();
    REQUIRE(result2.has_value() == false);
}

TEST_CASE("Vec front_or", "[front_or]") {
    Vec v = {1, 2, 3, 4, 5};

    // Test front_or() when the vector is not empty
    int result1 = v.front_or(0);
    REQUIRE(result1 == 1);

    // Test front_or() when the vector is empty
    Vec<int> emptyVec;
    int result2 = emptyVec.front_or(100);
    REQUIRE(result2 == 100);
}

TEST_CASE("Vec max and min", "[max][min]") {
    Vec v = {5, 2, 8, 1, 9};

    // Test finding the maximum value
    int maxVal = v.max();
    REQUIRE(maxVal == 9);

    // Test finding the minimum value
    int minVal = v.min();
    REQUIRE(minVal == 1);
}

//TEST_CASE("Vec histogram", "[histogram]") {
//    Vec<int> v = {1, 2, 2, 3, 3, 3, 4, 4, 4, 4};
//
//    // Test creating a histogram with default parameters
//    Vec hist = v.histogram(5);
//    REQUIRE(hist.size() == 5);
//    REQUIRE(hist[0] == 1);  // bin 1: [1]
//    REQUIRE(hist[1] == 2);  // bin 2: [2, 2]
//    REQUIRE(hist[2] == 3);  // bin 3: [3, 3, 3]
//    REQUIRE(hist[3] == 4);  // bin 4: [4, 4, 4, 4]
//    REQUIRE(hist[4] == 0);  // bin 5: []
//
//    // Test creating a histogram with specified thresholds
//    Vec histWithThresholds = v.histogram(5, {2}, {4});
//    REQUIRE(histWithThresholds.size() == 5);
//    REQUIRE(histWithThresholds[0] == 2);  // bin 1: [2, 2]
//    REQUIRE(histWithThresholds[1] == 3);  // bin 2: [3, 3, 3]
//    REQUIRE(histWithThresholds[2] == 4);  // bin 3: [4, 4, 4, 4]
//    REQUIRE(histWithThresholds[3] == 0);  // bin 4: []
//    REQUIRE(histWithThresholds[4] == 0);  // bin 5: []
//}

TEST_CASE("Vec sort", "[sort]") {
    Vec v = {5, 2, 8, 1, 9};

    // Test sorting in ascending order
    v.sort(true);
    REQUIRE(v.size() == 5);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);
    REQUIRE(v[2] == 5);
    REQUIRE(v[3] == 8);
    REQUIRE(v[4] == 9);

    // Test sorting in descending order
    v.sort(false);
    REQUIRE(v.size() == 5);
    REQUIRE(v[0] == 9);
    REQUIRE(v[1] == 8);
    REQUIRE(v[2] == 5);
    REQUIRE(v[3] == 2);
    REQUIRE(v[4] == 1);
}

TEST_CASE("Vec sum", "[sum]") {
    Vec v = {1, 2, 3, 4, 5};

    // Test summing the elements
    int sumResult = v.sum();
    REQUIRE(sumResult == 15);
}

TEST_CASE("Vec cumsum", "[cumsum]") {
    Vec v = {1, 2, 3, 4, 5};

    // Test computing cumulative sum
    int cumsumResult = v.cumsum();
    REQUIRE(cumsumResult == 15);
}

TEST_CASE("Vec mean", "[mean]") {
    Vec v = {1, 2, 3, 4, 5};

    // Test computing the mean
    int meanResult = v.mean();
    REQUIRE(meanResult == 3);
}

TEST_CASE("Vec pow", "[pow]") {
    Vec v = {1.0, 2.0, 3.0, 4.0, 5.0};
    v.pow(2.0);
    REQUIRE(v.size() == 5);
    REQUIRE_THAT(v[0], Catch::Matchers::WithinRel(1.0, 0.001));
    REQUIRE_THAT(v[1], Catch::Matchers::WithinRel(4.0, 0.001));
    REQUIRE_THAT(v[2], Catch::Matchers::WithinRel(9.0, 0.001));
    REQUIRE_THAT(v[3], Catch::Matchers::WithinRel(16.0, 0.001));
    REQUIRE_THAT(v[4], Catch::Matchers::WithinRel(25.0, 0.001));

}

TEST_CASE("Vec resize_extend", "[resize_extend]") {
    Vec v = {1, 2, 3};

    // Test resizing to extend with the last element
    v.resize_extend(5);
    REQUIRE(v.size() == 5);
    REQUIRE(v[3] == 3);
    REQUIRE(v[4] == 3);
}

TEST_CASE("Vec apply with indices", "[apply]") {
    Vec v = {1, 2, 3, 4, 5};
    Vec values = {10, 20, 30};
    Vec indices = {1, 3, 4};

    // Test applying a function to selected elements using indices
    v.apply([](int a, int b) { return a * b; }, values, indices);
    REQUIRE(v.size() == 5);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 20);
    REQUIRE(v[2] == 3);
    REQUIRE(v[3] == 80);
    REQUIRE(v[4] == 150);
}

TEST_CASE("Vec apply with binary mask", "[apply]") {
    Vec v = {1, 2, 3, 4, 5};
    Vec mask = {true, false, true, false, true};

    // Test applying a function to selected elements based on a binary mask
    v.apply([](int a, int b) { return a * b; }, 10, mask);
    REQUIRE(v.size() == 5);
    REQUIRE(v[0] == 10);
    REQUIRE(v[1] == 2);
    REQUIRE(v[2] == 30);
    REQUIRE(v[3] == 4);
    REQUIRE(v[4] == 50);
}

TEST_CASE("Vec clip", "[clip]") {
    Vec v = {1, 2, 3, 4, 5};

    // Test clipping values in the vector
    v.clip({2}, {4});
    REQUIRE(v.size() == 5);
    REQUIRE(v[0] == 2);
    REQUIRE(v[1] == 2);
    REQUIRE(v[2] == 3);
    REQUIRE(v[3] == 4);
    REQUIRE(v[4] == 4);
}

TEST_CASE("Vec cloned", "[cloned]") {
    Vec v = {1, 2, 3};

    // Test cloning the vector
    Vec clone = v.cloned();
    REQUIRE(clone.size() == 3);
    REQUIRE(clone[0] == 1);
    REQUIRE(clone[1] == 2);
    REQUIRE(clone[2] == 3);
}

TEST_CASE("Vec cloned does not mutate the original - part 1", "[cloned]") {
    Vec v = {1, 2, 3};
    Vec clone = v.cloned();

    // Modify the clone
    clone[0] = 10;

    // Ensure the original vector remains unchanged
    REQUIRE(v.size() == 3);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);
    REQUIRE(v[2] == 3);

    // Ensure the clone has been modified
    REQUIRE(clone.size() == 3);
    REQUIRE(clone[0] == 10);
    REQUIRE(clone[1] == 2);
    REQUIRE(clone[2] == 3);
}

TEST_CASE("Vec cloned does not mutate the original - part 2", "[cloned]") {
    Vec<std::string> v = {"apple", "banana", "cherry"};
    Vec<std::string> clone = v.cloned();

    // Modify the clone
    clone[1] = "grape";

    // Ensure the original vector remains unchanged
    REQUIRE(v.size() == 3);
    REQUIRE(v[0] == "apple");
    REQUIRE(v[1] == "banana");
    REQUIRE(v[2] == "cherry");

    // Ensure the clone has been modified
    REQUIRE(clone.size() == 3);
    REQUIRE(clone[0] == "apple");
    REQUIRE(clone[1] == "grape");
    REQUIRE(clone[2] == "cherry");
}


TEST_CASE("Function chaining") {
    Vec v = {1, 2, 3};
    REQUIRE_THAT(v.as_type<double>().pow(2.0).add(10).multiply(2).sum(), Catch::Matchers::WithinRel(88.0, 0.001));
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);
    REQUIRE(v[2] == 3);
}


class Temp {
};

TEST_CASE("Non-arithmethic initialization") {
    Vec v{Temp(), Temp(), Temp()};
}