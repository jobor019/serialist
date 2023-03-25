#include <catch2/catch_test_macros.hpp>


TEST_CASE("dummy test2", "[dummytag1, dummytag2]") {
    REQUIRE(1 == 1);
}


TEST_CASE("dummy test2", "[dummytag3]") {
    REQUIRE(0 < 1);
}