#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/utility/math.h"


TEST_CASE("double2index tests") {
    REQUIRE(utils::double2index(0.0, 5) == 0);
    REQUIRE(utils::double2index(0.2, 5) == 1);
    REQUIRE(utils::double2index(0.4, 5) == 2);
    REQUIRE(utils::double2index(0.6, 5) == 3);
    REQUIRE(utils::double2index(0.8, 5) == 4);

    REQUIRE(utils::double2index(-0.2, 5) == 4); // equiv index -1
    REQUIRE(utils::double2index(-0.4, 5) == 3); // equiv index -2
    REQUIRE(utils::double2index(-0.6, 5) == 2); // ...
    REQUIRE(utils::double2index(-0.8, 5) == 1);

}