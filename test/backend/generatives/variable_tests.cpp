#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/generatives/variable.h"
#include "core/algo/facet.h"


TEST_CASE("Variable initialization") {
    ParameterHandler handler;

    auto var = Variable<Facet, int>("", handler, 123);
}
