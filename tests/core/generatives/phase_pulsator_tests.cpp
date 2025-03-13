
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/policies/policies.h"
#include "node_runner.h"
#include "core/generatives/phase_pulsator.h"
#include "generatives/oscillator.h"

#include "generators.h"
#include "matchers/m1m.h"
#include "matchers/m11.h"

using namespace serialist;
using namespace serialist::test;


TEST_CASE("PhasePulsator: TEMP SANITY CHECK TEST (Requirements not formalized", "[phase_pulsator]") {
    auto p = PhasePulsatorWrapper();

    p.legato_amount.set_values(1.0);

    auto o = OscillatorWrapper();
    o.period.set_values(1.0);
    o.period_type.set_value(DomainType::ticks);
    o.offset.set_values(0.0);

    p.pulsator_node.set_cursor(&o.oscillator);

    NodeRunner runner{&p.pulsator_node};
    runner.add_generative(o.oscillator);
    auto time_epsilon = runner.get_config().step_size.get_value() + EPSILON;

    std::cout << "======== STEP 0 ========\n";
    auto r = runner.step();


    REQUIRE_THAT(r, m1m::equalst_on());
    auto pulse_on_id = *r.pulse_on_id();


    std::cout << "======== STEP 1 ======== \n";
    r = runner.step_while(c1m::emptyt());
    // r = runner.step();

    r.print();
    o.oscillator.process().print();
    REQUIRE_THAT(r, m1m::containst_off(pulse_on_id));
    REQUIRE_THAT(r, m1m::containst_on());
    REQUIRE_THAT(r, m1m::sizet(2));

    REQUIRE_THAT(r.time(), TimePointMatcher(1.0).with_epsilon(time_epsilon));
    pulse_on_id = *r.pulse_on_id();

    r = runner.step_while(c1m::emptyt());
    // r = runner.step();

    r.print();
    o.oscillator.process().print();
    REQUIRE_THAT(r, m1m::containst_off(pulse_on_id));
    REQUIRE_THAT(r, m1m::containst_on());
    REQUIRE_THAT(r, m1m::sizet(2));

    REQUIRE_THAT(r.time(), TimePointMatcher(2.0).with_epsilon(time_epsilon));

    r.print_all();
    std::cout << r.num_steps() << "\n";
}
