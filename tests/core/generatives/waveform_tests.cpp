

#include "core/policies/policies.h"
#include "node_runner.h"
#include "generatives/waveform.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>


#include "generators.h"
#include "matchers/m1m.h"
#include "matchers/m11.h"
#include "matchers/m1s.h"
#include "matchers/mms.h"

using namespace serialist;
using namespace serialist::test;

TEST_CASE("Waveform: ctor", "[waveform]") {
    WaveformWrapper<> w;

    w.trigger.set_values(Trigger::pulse_on());
    w.phase.set_values(0.0);

    NodeRunner runner{&w.waveform};

    auto r = runner.step();
    REQUIRE_THAT(r, m11::eqf(0.0));
}