
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/algo/classifiers.h"

using namespace serialist;


TEST_CASE("LinearBandClassifier<int> Tests", "[LinearBandClassifier<int>]") {
    LinearBandClassifier classifier(0, 99, 10);

    SECTION("Classify Values") {
        REQUIRE(classifier.classify(5) == 0);
        REQUIRE(classifier.classify(25) == 2);
        REQUIRE(classifier.classify(105) == 9);
        REQUIRE(classifier.classify(-5) == 0);
    }

    SECTION("Get Number of Bands") {
        REQUIRE(classifier.get_num_classes() == 10);
    }

    SECTION("Start and End of Bands") {
        REQUIRE(classifier.start_of(0) == 0);
        REQUIRE(classifier.end_of(0) == 10);
        REQUIRE(classifier.start_of(1) == 10);
        REQUIRE(classifier.end_of(1) == 20);
        REQUIRE(classifier.start_of(3) == 30);
        REQUIRE(classifier.end_of(3) == 40);
        REQUIRE(classifier.start_of(9) == 90);
        REQUIRE(classifier.end_of(9) == 99);
    }
}


TEST_CASE("LinearBandClassifier<double> Tests", "[LinearBandClassifier<double>]") {
    LinearBandClassifier classifier(0.0, 1.0, 10);

    SECTION("Classify Values") {
        REQUIRE(classifier.classify(0.05) == 0);
        REQUIRE(classifier.classify(0.25) == 2);
        REQUIRE(classifier.classify(1.05) == 9);
        REQUIRE(classifier.classify(-0.05) == 0);
    }

    SECTION("Get Number of Bands") {
        REQUIRE(classifier.get_num_classes() == 10);
    }

    SECTION("Start and End of Bands") {
        REQUIRE_THAT(classifier.start_of(3), Catch::Matchers::WithinRel(0.3, 1e-6));
        REQUIRE_THAT(classifier.end_of(5), Catch::Matchers::WithinRel(0.6, 1e-6));
        REQUIRE_THAT(classifier.start_of(9), Catch::Matchers::WithinRel(0.9, 1e-6));
        REQUIRE_THAT(classifier.end_of(9), Catch::Matchers::WithinRel(1.0, 1e-6));
    }
}


TEST_CASE("LinearBandClassifier<int> Additional Tests", "[LinearBandClassifier<int>]") {
    SECTION("LinearSpace: 10-59, Band Width: 10") {
        LinearBandClassifier classifier(10, 59, 5);
        REQUIRE(classifier.get_num_classes() == 5);

        REQUIRE(classifier.classify(15) == 0);
        REQUIRE(classifier.classify(20) == 1);
        REQUIRE(classifier.classify(50) == 4);

        REQUIRE(classifier.start_of(3) == 40);
        REQUIRE(classifier.end_of(3) == 50);
        REQUIRE(classifier.start_of(4) == 50);
        REQUIRE(classifier.end_of(4) == 59);
    }

    SECTION("LinearSpace: -30 to 60, Band Width: 15") {
        LinearBandClassifier classifier(-30, 60, 6);
        REQUIRE(classifier.get_num_classes() == 6);

        REQUIRE(classifier.classify(5) == 2);
        REQUIRE(classifier.classify(-20) == 0);
        REQUIRE(classifier.classify(90) == 5);

        REQUIRE(classifier.start_of(1) == -15);
        REQUIRE(classifier.end_of(1) == 0);
        REQUIRE(classifier.start_of(5) == 45);
        REQUIRE(classifier.end_of(5) == 60);
    }
}
