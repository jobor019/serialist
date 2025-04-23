#include "core/policies/policies.h"
#include "node_runner.h"
#include "generatives/router.h"

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

TEST_CASE("Router: ctor", "[router]") {
    RouterFacetWrapper w(3, 3);

    w.set_input(0, Voices<double>::singular(0));
    w.routing_map.set_values(0.0);
    w.uses_index.set_value(true);
    w.router_node.update_time(TimePoint::zero());

    auto s = w.router_node.process();
    for (auto& o : s) {
        o.print();
    }
}
