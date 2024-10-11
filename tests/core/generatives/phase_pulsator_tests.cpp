
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/policies/policies.h"
#include "core/generatives/phase_pulsator.h"

using namespace serialist;


TEST_CASE("PhasePulsator ctors") {
    PhasePulsator p;
    PhasePulsatorWrapper<> w;
}