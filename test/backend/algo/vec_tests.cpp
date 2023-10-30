
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/collections/vec.h"

TEST_CASE("Vec Constructors", "[constructor]") {
    Vec<int> v1;
    REQUIRE(v1.empty());

    Vec v2({1, 2, 3});
    REQUIRE(v2.size() == 3);
    REQUIRE(v2[0] == 1);
    REQUIRE(v2[1] == 2);
    REQUIRE(v2[2] == 3);

    Vec v3 = {4, 5, 6};
    REQUIRE(v3.size() == 3);
    REQUIRE(v3[0] == 4);
    REQUIRE(v3[1] == 5);
    REQUIRE(v3[2] == 6);
}


TEST_CASE("Vec operator[]", "[operator]") {
    SECTION("Accessing element") {
        Vec v1({1, 2, 3});
        REQUIRE(v1[0] == 1);
        REQUIRE(v1[1] == 2);
        REQUIRE(v1[2] == 3);
    }

    SECTION("Accessing out of bounds") {
        Vec v1({1, 2, 3});
        REQUIRE_THROWS(v1[3]);
    }

    SECTION("Reassignment") {
        Vec v1({1, 2, 3});
        v1[0] = 4;
        REQUIRE(v1[0] == 4);
    }
}


TEST_CASE("Vec operator==", "[operator]") {
    Vec v1({1, 2, 3});
    Vec v2({1, 2, 3});
    Vec v3({4, 5, 6});

    REQUIRE(v1 == v2);

    REQUIRE_FALSE(v1 == v3);
}


TEST_CASE("Vec approx_equals", "[approx_equals]") {
    Vec v1({1.0, 2.0, 3.0});
    auto v2 = Vec({1.0, 2.0, 3.0}).add(1e-7);
    Vec v3({1.1, 2.1, 2.9});
    Vec v4({4.0, 5.0, 6.0});

    REQUIRE(v1.approx_equals(v2));
    REQUIRE_FALSE(v1.approx_equals(v3));
    REQUIRE_FALSE(v1.approx_equals(v4));

    double epsilon = 1.0;
    REQUIRE(v1.approx_equals(v2, epsilon));
    REQUIRE(v1.approx_equals(v3, epsilon));
    REQUIRE_FALSE(v1.approx_equals(v4, epsilon));
}


TEST_CASE("Vec operator+", "[operator]") {
    Vec v1({1, 2, 3});
    Vec v2({4, 5, 6});

    SECTION("Vector addition") {
        Vec result = v1 + v2;
        REQUIRE(v1 == Vec({1, 2, 3}));
        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == 5);
        REQUIRE(result[1] == 7);
        REQUIRE(result[2] == 9);
    }

    SECTION("Scalar addition") {
        Vec result = v1 + 5;
        REQUIRE(v1 == Vec({1, 2, 3}));
        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == 6);
        REQUIRE(result[1] == 7);
        REQUIRE(result[2] == 8);
    }

    SECTION("Mixed addition") {
        Vec result = v1 + 10 + v2 + 100;
        REQUIRE(v1 == Vec({1, 2, 3}));
        REQUIRE(v2 == Vec({4, 5, 6}));
        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == 115);
        REQUIRE(result[1] == 117);
        REQUIRE(result[2] == 119);
    }
}



TEST_CASE("Vec range", "[range]") {
    Vec<int> v0 = Vec<int>::range(2, 4);
    REQUIRE(v0.size() == 2);
    REQUIRE(v0[0] == 2);
    REQUIRE(v0[1] == 3);

    Vec<int> v1 = Vec<int>::range(1, 6, 2);
    REQUIRE(v1.size() == 3);
    REQUIRE(v1[0] == 1);
    REQUIRE(v1[1] == 3);
    REQUIRE(v1[2] == 5);

    Vec<double> v2 = Vec<double>::range(1.0, 4.0, 0.5);
    REQUIRE(v2.size() == 6);
    REQUIRE(v2[0] == 1.0);
    REQUIRE(v2[1] == 1.5);
    REQUIRE(v2[5] == 3.5);
}


TEST_CASE("Vec repeated", "[repeated]") {
    Vec<int> v1 = Vec<int>::repeated(3, 5);
    REQUIRE(v1.size() == 3);
    REQUIRE(v1[0] == 5);
    REQUIRE(v1[1] == 5);
    REQUIRE(v1[2] == 5);

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

    v1.concatenate(v2);
    REQUIRE(v1.size() == 4);
    REQUIRE(v1[0] == 1);
    REQUIRE(v1[1] == 2);
    REQUIRE(v1[2] == 3);
    REQUIRE(v1[3] == 4);
}


TEST_CASE("Vec remove", "[remove]") {
    Vec v = {1, 2, 3, 4, 5};

    v.remove(3);
    REQUIRE(v.size() == 4);
    REQUIRE_FALSE(v.contains(3));

    v.remove(6);
    REQUIRE(v.size() == 4);
}


TEST_CASE("Vec resize_append", "[resize_append]") {
    Vec v = {1, 2, 3};

    v.resize_append(2, 0);
    REQUIRE(v.size() == 2);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);

    v.resize_append(5, 10);
    REQUIRE(v.size() == 5);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);
    REQUIRE(v[2] == 10);
    REQUIRE(v[3] == 10);
    REQUIRE(v[4] == 10);
}


TEST_CASE("Vec resize_fold", "[resize_fold]") {
    Vec v = {1, 2, 3};

    v.resize_fold(4);
    REQUIRE(v.size() == 4);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);
    REQUIRE(v[2] == 3);
    REQUIRE(v[3] == 1);

    v.resize_fold(2);
    REQUIRE(v.size() == 2);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);
}

TEST_CASE("Vec resize_default", "[resize_default]") {
    Vec v = {1, 2, 3};

    v.resize_default(4);
    REQUIRE(v.size() == 4);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);
    REQUIRE(v[2] == 3);
    REQUIRE(v[3] == 0);

    v.resize_fold(2);
    REQUIRE(v.size() == 2);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);
}


TEST_CASE("Vec erase", "[erase]") {
    Vec v = {1, 2, 3, 4, 5};

    v.erase(2);
    REQUIRE(v.size() == 4);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);
    REQUIRE(v[2] == 4);
    REQUIRE(v[3] == 5);

    v.erase(1, 3);
    REQUIRE(v.size() == 2);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 5);
}


TEST_CASE("Vec slice", "[slice]") {
    Vec v = {1, 2, 3, 4, 5};

    Vec<int> sliced = v.slice(0, 3);
    REQUIRE(sliced.size() == 3);
    REQUIRE(sliced[0] == 1);
    REQUIRE(sliced[1] == 2);
    REQUIRE(sliced[2] == 3);

    sliced = v.slice(2);
    REQUIRE(sliced.size() == 2);
    REQUIRE(sliced[0] == 3);
    REQUIRE(sliced[1] == 4);

    sliced = v.slice(-3, -1);
    REQUIRE(sliced.size() == 2);
    REQUIRE(sliced[0] == 3);
    REQUIRE(sliced[1] == 4);
}


TEST_CASE("Vec as_type", "[as_type]") {
    Vec v = {1.5, 2.5, 3.5};

    Vec<int> converted = v.as_type<int>();
    REQUIRE(converted.size() == 3);
    REQUIRE(converted[0] == 1);
    REQUIRE(converted[1] == 2);
    REQUIRE(converted[2] == 3);
}


TEST_CASE("Vec as_type with lambda", "[as_type]") {
    Vec v = {1, 2, 3}; // Specify the template argument here

    Vec<std::string> converted = v.as_type<std::string>([](const auto& s) { return std::to_string(s); });
    REQUIRE(converted.size() == 3);
    REQUIRE(converted[0] == "1");
    REQUIRE(converted[1] == "2");
    REQUIRE(converted[2] == "3");
}


TEST_CASE("Vec apply", "[apply]") {
    Vec v = {1, 2, 3, 4, 5};

    v.apply([](int x, int y) { return x + y; }, 10);
    REQUIRE(v.size() == 5);
    REQUIRE(v[0] == 11);
    REQUIRE(v[1] == 12);
    REQUIRE(v[2] == 13);
    REQUIRE(v[3] == 14);
    REQUIRE(v[4] == 15);

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

    REQUIRE(v.contains(3) == true);

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

    const std::vector<int>& constVector = v.vector();
    REQUIRE(constVector.size() == 5);
    REQUIRE(constVector[0] == 1);

    std::vector<int>& mutableVector = v.vector_mut();
    mutableVector.push_back(6);
    REQUIRE(v.size() == 6);
    REQUIRE(v[5] == 6);
}


TEST_CASE("Vec first", "[first]") {
    Vec v = {1, 2, 3, 4, 5};

    std::optional<int> result1 = v.first();
    REQUIRE(result1.has_value() == true);
    REQUIRE(result1.value() == 1);

    Vec<int> emptyVec;
    std::optional<int> result2 = emptyVec.first();
    REQUIRE(result2.has_value() == false);
}


TEST_CASE("Vec first_or", "[first_or]") {
    Vec v = {1, 2, 3, 4, 5};

    int result1 = v.first_or(0);
    REQUIRE(result1 == 1);

    Vec<int> emptyVec;
    int result2 = emptyVec.first_or(100);
    REQUIRE(result2 == 100);
}


TEST_CASE("Vec max and min", "[max][min]") {
    Vec v = {5, 2, 8, 1, 9};

    int maxVal = v.max();
    REQUIRE(maxVal == 9);

    int minVal = v.min();
    REQUIRE(minVal == 1);
}


TEST_CASE("Vec sort", "[sort]") {
    Vec v = {5, 2, 8, 1, 9};

    v.sort(true);
    REQUIRE(v.size() == 5);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);
    REQUIRE(v[2] == 5);
    REQUIRE(v[3] == 8);
    REQUIRE(v[4] == 9);

    v.sort(false);
    REQUIRE(v.size() == 5);
    REQUIRE(v[0] == 9);
    REQUIRE(v[1] == 8);
    REQUIRE(v[2] == 5);
    REQUIRE(v[3] == 2);
    REQUIRE(v[4] == 1);
}


TEST_CASE("Vec reorder", "[reorder]") {
    Vec v({5, 2, 8, 1, 9});

    SECTION("Reordering with valid indices") {
        Vec<std::size_t> indices({3, 1, 0, 2, 4});
        v.reorder(indices);

        REQUIRE(v.size() == 5);
        REQUIRE(v[0] == 1);
        REQUIRE(v[1] == 2);
        REQUIRE(v[2] == 5);
        REQUIRE(v[3] == 8);
        REQUIRE(v[4] == 9);
    }

    SECTION("Reordering with invalid indices") {
        Vec<std::size_t> invalid_indices({1, 2, 3});

        REQUIRE_THROWS_AS(v.reorder(invalid_indices), std::out_of_range);
    }
}


TEST_CASE("Vec argsort", "[argsort]") {
    Vec v({5, 2, 8, 1, 9});

    SECTION("Argsort in ascending order without applying the sort") {
        Vec<std::size_t> ascending_indices = v.argsort(true, false);

        REQUIRE(ascending_indices.size() == 5);
        REQUIRE(ascending_indices[0] == 3);
        REQUIRE(ascending_indices[1] == 1);
        REQUIRE(ascending_indices[2] == 0);
        REQUIRE(ascending_indices[3] == 2);
        REQUIRE(ascending_indices[4] == 4);
    }

    SECTION("Argsort in descending order and apply the sort to the vector") {
        Vec<std::size_t> descending_indices = v.argsort(false, true);

        REQUIRE(descending_indices.size() == 5);
        REQUIRE(descending_indices[0] == 4);
        REQUIRE(descending_indices[1] == 2);
        REQUIRE(descending_indices[2] == 0);
        REQUIRE(descending_indices[3] == 1);
        REQUIRE(descending_indices[4] == 3);

        REQUIRE(v[0] == 9);
        REQUIRE(v[1] == 8);
        REQUIRE(v[2] == 5);
        REQUIRE(v[3] == 2);
        REQUIRE(v[4] == 1);
    }
}


TEST_CASE("Vec sum", "[sum]") {
    Vec v = {1, 2, 3, 4, 5};

    int sumResult = v.sum();
    REQUIRE(sumResult == 15);
}


TEST_CASE("Vec cumsum", "[cumsum]") {
    Vec v = {1, 2, 3, 4, 5};

    REQUIRE(v.cumsum() == Vec{1, 3, 6, 10, 15});
}


TEST_CASE("Vec mean", "[mean]") {
    Vec v = {1, 2, 3, 4, 5};

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

    v.resize_extend(5);
    REQUIRE(v.size() == 5);
    REQUIRE(v[3] == 3);
    REQUIRE(v[4] == 3);
}


TEST_CASE("Vec apply with indices", "[apply]") {
    Vec v = {1, 2, 3, 4, 5};
    Vec values = {10, 20, 30};
    Vec indices = {1, 3, 4};

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

    SECTION("Double bounds") {
        v.clip({2}, {4});
        REQUIRE(v.size() == 5);
        REQUIRE(v[0] == 2);
        REQUIRE(v[1] == 2);
        REQUIRE(v[2] == 3);
        REQUIRE(v[3] == 4);
        REQUIRE(v[4] == 4);
    }

    SECTION("lower bound") {
        v.clip({4}, std::nullopt);
        REQUIRE(v.size() == 5);
        REQUIRE(v[0] == 4);
        REQUIRE(v[1] == 4);
        REQUIRE(v[2] == 4);
        REQUIRE(v[3] == 4);
        REQUIRE(v[4] == 5);
    }

    SECTION("upper bound") {
        v.clip(std::nullopt, {3});
        REQUIRE(v.size() == 5);
        REQUIRE(v[0] == 1);
        REQUIRE(v[1] == 2);
        REQUIRE(v[2] == 3);
        REQUIRE(v[3] == 3);
        REQUIRE(v[4] == 3);
    }

    SECTION("out of bounds") {
        v.clip({8}, {11});
        REQUIRE(v.size() == 5);
        REQUIRE(v[0] == 8);
        REQUIRE(v[1] == 8);
        REQUIRE(v[2] == 8);
        REQUIRE(v[3] == 8);
        REQUIRE(v[4] == 8);
    }
}


TEST_CASE("Vec cloned", "[cloned]") {
    Vec v = {1, 2, 3};

    Vec clone = v.cloned();
    REQUIRE(clone.size() == 3);
    REQUIRE(clone[0] == 1);
    REQUIRE(clone[1] == 2);
    REQUIRE(clone[2] == 3);
}


TEST_CASE("Vec cloned does not mutate the original - part 1", "[cloned]") {
    Vec v = {1, 2, 3};
    Vec clone = v.cloned();

    clone[0] = 10;

    REQUIRE(v.size() == 3);
    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);
    REQUIRE(v[2] == 3);

    REQUIRE(clone.size() == 3);
    REQUIRE(clone[0] == 10);
    REQUIRE(clone[1] == 2);
    REQUIRE(clone[2] == 3);
}


TEST_CASE("Vec cloned does not mutate the original - part 2", "[cloned]") {
    Vec<std::string> v = {"apple", "banana", "cherry"};
    Vec<std::string> clone = v.cloned();

    clone[1] = "grape";

    REQUIRE(v.size() == 3);
    REQUIRE(v[0] == "apple");
    REQUIRE(v[1] == "banana");
    REQUIRE(v[2] == "cherry");

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


TEST_CASE("Test map function", "[map]") {
    Vec v({1, 2, 3, 4, 5});

    auto double_fn = [](int x) { return x * 2; };

    v.map(double_fn);

    REQUIRE(v.vector() == std::vector<int>({2, 4, 6, 8, 10}));
}


TEST_CASE("Test filter function", "[filter]") {
    Vec v({1, 2, 3, 4, 5});

    auto even_fn = [](int x) { return x % 2 == 0; };

    v.filter(even_fn);

    REQUIRE(v.vector() == std::vector<int>({2, 4}));
}


TEST_CASE("Test foldl function", "[foldl]") {
    Vec v({1, 2, 3, 4, 5});
    auto sum_fn = [](int x, int y) { return x + y; };
    int result = v.foldl(sum_fn, 0);
    REQUIRE(result == 15);
}


TEST_CASE("Test filter_drain function", "[filter_drain]") {
    Vec v({1, 2, 3, 4, 5});
    auto even_fn = [](int x) { return x % 2 == 0; };
    Vec<int> drained = v.filter_drain(even_fn);

    REQUIRE(v.vector() == std::vector<int>({2, 4}));
    REQUIRE(drained.vector() == std::vector<int>({1, 3, 5}));
}


TEST_CASE("Vec normalize_max", "[normalize_max]") {
    Vec<float> v2 = {2.0, 4.0, 10.0};
    v2.normalize_max();

    REQUIRE(v2.size() == 3);
    REQUIRE_THAT(v2[0], Catch::Matchers::WithinRel(0.2, 1e-6));
    REQUIRE_THAT(v2[1], Catch::Matchers::WithinRel(0.4, 1e-6));
    REQUIRE_THAT(v2[2], Catch::Matchers::WithinRel(1.0, 1e-6));
}


TEST_CASE("Vec normalize_l1", "[normalize_l1]") {
    Vec<float> v2 = {1.0, 2.0, 3.0, 4.0};
    v2.normalize_l1();

    REQUIRE(v2.size() == 4);
    REQUIRE_THAT(v2[0], Catch::Matchers::WithinRel(0.1, 1e-6));
    REQUIRE_THAT(v2[1], Catch::Matchers::WithinRel(0.2, 1e-6));
    REQUIRE_THAT(v2[2], Catch::Matchers::WithinRel(0.3, 1e-6));
    REQUIRE_THAT(v2[3], Catch::Matchers::WithinRel(0.4, 1e-6));
}


TEST_CASE("Vec normalize_l2", "[normalize_l2]") {
    Vec<float> v2 = {1.0, 2.0, 3.0, 4.0};
    v2.normalize_l2();

    REQUIRE(v2.size() == 4);
    REQUIRE_THAT(v2[0], Catch::Matchers::WithinRel(0.18257419, 1e-6));
    REQUIRE_THAT(v2[1], Catch::Matchers::WithinRel(0.36514837, 1e-6));
    REQUIRE_THAT(v2[2], Catch::Matchers::WithinRel(0.54772256, 1e-6));
    REQUIRE_THAT(v2[3], Catch::Matchers::WithinRel(0.73029674, 1e-6));
}


TEST_CASE("Test extend function", "[extend]") {
    Vec v1({1, 2, 3});
    Vec v2({4, 5, 6});
    v1.extend(v2);

    REQUIRE(v1.vector() == std::vector<int>({1, 2, 3, 4, 5, 6}));
}


TEST_CASE("Test linspace function") {
    SECTION("Integral linspaces") {
        auto v0 = Vec<int>::linspace(0, 10, 0);
        REQUIRE(v0.empty());

        auto v1 = Vec<int>::linspace(0, 10, 11);
        REQUIRE(v1 == Vec({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));

        auto v2 = Vec<int>::linspace(0, 10, 11, false);
        REQUIRE(v2 == Vec({0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9}));

        auto v3 = Vec<int>::linspace(0, 10, 3);
        REQUIRE(v3 == Vec({0, 5, 10}));

        auto v4 = Vec<int>::linspace(0, 10, 3, false);
        REQUIRE(v4 == Vec({0, 3, 6}));

        auto v5 = Vec<int>::linspace(10, 0, 11);
        REQUIRE(v5 == Vec({10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0}));

        auto v6 = Vec<int>::linspace(10, 0, 11, false);
        REQUIRE(v6 == Vec({10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0}));

        auto v8 = Vec<int>::linspace(10, 0, 3);
        REQUIRE(v8 == Vec({10, 5, 0}));

        auto v9 = Vec<int>::linspace(10, 0, 3, false);
        REQUIRE(v9 == Vec({10, 6, 3}));
    }

    SECTION("Float-point linspaces") {
        auto v0 = Vec<double>::linspace(0.0, 10.0, 0);
        REQUIRE(v0.empty());

        auto v1 = Vec<double>::linspace(0.0, 10.0, 11);
        REQUIRE(v1.approx_equals(Vec<double>({0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0}), 1e-3));

        auto v2 = Vec<double>::linspace(0.0, 10.0, 11, false);
        REQUIRE(v2.approx_equals(Vec<double>({0.0, 0.9091, 1.8182, 2.7273, 3.6364, 4.5455
                                              , 5.4545, 6.3636, 7.2727, 8.1818, 9.0909}), 1e-3));

        auto v3 = Vec<double>::linspace(0.0, 10.0, 3);
        REQUIRE(v3.approx_equals(Vec<double>({0.0, 5.0, 10.0}), 1e-3));

        auto v4 = Vec<double>::linspace(0.0, 10.0, 3, false);
        REQUIRE(v4.approx_equals(Vec<double>({0.0, 3.3333, 6.6667}), 1e-3));

        auto v5 = Vec<double>::linspace(10.0, 0.0, 11);
        REQUIRE(v5.approx_equals(Vec<double>({10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0}), 1e-3));

        auto v6 = Vec<double>::linspace(10.0, 0.0, 11, false);
        REQUIRE(v6.approx_equals(Vec<double>({10.0, 9.0909, 8.1818, 7.2727, 6.3636, 5.4545
                                              , 4.5455, 3.6364, 2.7273, 1.8182, 0.9091}), 1e-3));

        auto v8 = Vec<double>::linspace(10.0, 0.0, 3);
        REQUIRE(v8.approx_equals(Vec<double>({10.0, 5.0, 0.0}), 1e-3));

        auto v9 = Vec<double>::linspace(10.0, 0.0, 3, false);
        REQUIRE(v9.approx_equals(Vec<double>({10.0, 6.6667, 3.3333}), 1e-3));
    }


}

//TEST_CASE("Test rotate function", "[rotate]") {
//    Vec v({1, 2, 3, 4, 5});
//
//    SECTION("Rotate right by 2 elements") {
//        v.rotate(2);
//        REQUIRE(v.vector() == std::vector<int>({4, 5, 1, 2, 3}));
//    }
//
//    SECTION("Rotate left by 1 element") {
//        v.rotate(-1);
//        REQUIRE(v.vector() == std::vector<int>({2, 3, 4, 5, 1}));
//    }
//
//    SECTION("No rotation (amount is 0)") {
//        v.rotate(0);
//        REQUIRE(v.vector() == std::vector<int>({1, 2, 3, 4, 5}));
//    }
//}


TEST_CASE("Test shift function", "[shift]") {
    Vec v({1, 2, 3, 4, 5});

    SECTION("Shift right by 2 elements") {
        v.shift(2);
        REQUIRE(v == Vec({0, 0, 1, 2, 3}));
    }

    SECTION("Shift left by 1 element") {
        v.shift(-1);
        REQUIRE(v == Vec({2, 3, 4, 5, 0}));
    }

    SECTION("Shift left by 5 elements (completely shift out)") {
        v.shift(-5);
        REQUIRE(v == Vec<int>::zeros(5));
    }

    SECTION("No shift (amount is 0)") {
        v.shift(0);
        REQUIRE(v == Vec({1, 2, 3, 4, 5}));
    }
}





class Temp {
};

TEST_CASE("Non-arithmethic initialization") {
    Vec v{Temp(), Temp(), Temp()};
}