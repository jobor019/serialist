#include <catch2/generators/catch_generators.hpp>

#include "core/policies/policies.h"
#include "node_runner.h"
#include "algo/facet.h"
#include "generatives/sequence.h"
#include "generatives/variable.h"
#include "generatives/stereotypes/base_stereotypes.h"
using namespace serialist;
using namespace serialist::test;

/** Dummy node that returns current time as a facet (independently of number of voices) */
class DummyNode : public NodeBase<Facet> {
public:
    DummyNode(DomainType domain_type = DomainType::ticks)
    :  NodeBase("dummy", m_ph, &m_enabled, &m_num_voices, "DummyNode")
    , m_domain_type(domain_type) {}

    void update_time(const TimePoint& t) override {
        m_current_time = t;
    }


    Voices<Facet> process() override {
        return Voices<Facet>::singular(Facet(m_current_time.get(m_domain_type)));
    }

    void set_domain_type(DomainType domain_type) { m_domain_type = domain_type; }

private:
    DomainType m_domain_type;

    ParameterHandler m_ph = ParameterHandler();
    Sequence<Facet, bool> m_enabled{param::properties::enabled, m_ph, Voices<bool>::singular(true)};
    Variable<Facet, std::size_t> m_num_voices{param::properties::num_voices, m_ph, 0};

    TimePoint m_current_time = TimePoint{};
};


// ==============================================================================================

TEST_CASE("Dummy Node", "[node_runner]") {
    DummyNode node;
    node.update_time(TimePoint(0.0));

    auto v = node.process().first();
    REQUIRE(v.has_value());
    REQUIRE(utils::equals(static_cast<double>(*v), 0.0));

    node.update_time(TimePoint(100.0));
    v = node.process().first();
    REQUIRE(v.has_value());
    REQUIRE(utils::equals(static_cast<double>(*v), 100.0));
}


// ==============================================================================================

TEST_CASE("NodeRunner: step_until", "[node_runner]") {
    DummyNode node;

    double step_size = GENERATE(0.1, 0.999);
    double target_time = GENERATE(1.0, 10.0, 100.0);
    DomainType domain_type = GENERATE(DomainType::ticks, DomainType::beats, DomainType::bars);
    CAPTURE(target_time, step_size);

    node.set_domain_type(domain_type);

    auto config = TestConfig().with_step_size(DomainDuration(step_size)).with_history_capacity(0);
    NodeRunner runner{&node, config};

    SECTION("Stop::after") {
        for (int i = 0; i < 10; ++i) {
            auto t = target_time * (i + 1);
            auto r = runner.step_until(DomainTimePoint(t, domain_type), Stop::after);

            CAPTURE(i, t, r);
            REQUIRE(r.is_successful());

            REQUIRE(utils::in(*r.v11f(),  t, t + step_size));
        }
    }

    SECTION("Stop::before") {
        for (int i = 0; i < 10; ++i) {
            auto t = target_time * (i + 1);
            auto r = runner.step_until(DomainTimePoint(t, domain_type), Stop::before);

            CAPTURE(i, t, r);
            REQUIRE(r.is_successful());

            REQUIRE(utils::in(*r.v11f(),  t - step_size, t));
        }
    }
}


TEST_CASE("NodeRunner: step_until edge cases", "[node_runner]") {
    double start_time = 1.0;
    double step_size = 0.1;

    DummyNode node;
    auto config = TestConfig().with_step_size(DomainDuration(step_size)).with_history_capacity(0);

    TimePoint initial_time{start_time};
    NodeRunner runner{&node, config, initial_time};

    SECTION("Step size 0") {
        REQUIRE_THROWS(config.with_step_size(DomainDuration(0)));
    }

    SECTION("Run to previous time") {
        auto r = runner.step_until(DomainTimePoint(0.0, DomainType::ticks), Stop::before);
        CAPTURE(r);
        REQUIRE(!r.is_successful());
    }

    SECTION("Run to current time (Before)") {
        auto r = runner.step_until(DomainTimePoint(start_time, DomainType::ticks), Stop::before);
        CAPTURE(r);
        REQUIRE(!r.is_successful());
    }

    // Technically we should be able to run to current time (after) if it's the first step. TODO: Look into this
    // SECTION("Run to current time as first step (After)") {
    //     auto r = runner.step_until(DomainTimePoint(start_time, DomainType::ticks), Stop::after);
    //     CAPTURE(r);
    //     REQUIRE(r.is_successful());
    // }

    SECTION("Run to current time + step_size (After)") {
        auto r = runner.step_until(DomainTimePoint(start_time + step_size, DomainType::ticks), Stop::after);
        CAPTURE(r);
        REQUIRE(r.is_successful());
        REQUIRE(r.last().voices().first().value() >= start_time + step_size);
    }

    SECTION("Run to current time + step_size (Before)") {
        // On first run, this should be possible, as it will generate a single step at current time without incrementing
        auto r = runner.step_until(DomainTimePoint(start_time + step_size, DomainType::ticks), Stop::before);
        REQUIRE(r.is_successful());
        REQUIRE(r.num_steps() == 1);
    }

    SECTION("Run to given time twice (Before)") {
        auto r = runner.step_until(DomainTimePoint(2.0, DomainType::ticks), Stop::before);
        REQUIRE(r.is_successful());
        r = runner.step_until(DomainTimePoint(2.0, DomainType::ticks), Stop::before);
        CAPTURE(r);
        REQUIRE(!r.is_successful());
    }

    SECTION("Run to current time + step_size/2 (After)") {
        // Consume first step
        auto r = runner.step_until(DomainTimePoint(2.0, DomainType::ticks), Stop::after);
        REQUIRE(r.is_successful());
        r = runner.step_until(DomainTimePoint(2.0 + step_size/2, DomainType::ticks), Stop::after);
        CAPTURE(r);
        REQUIRE(r.is_successful());
    }

    SECTION("Consecutive steps (before)") {
        auto target = GENERATE(2.0, 3.75, 10.0);
        auto r = runner.step_until(DomainTimePoint(target, DomainType::ticks), Stop::before);
        REQUIRE(r.is_successful());
        REQUIRE(utils::in(*r.v11f(), target - step_size, target));

        r = runner.step();
        CAPTURE(r);
        REQUIRE(r.is_successful());
        REQUIRE(utils::in(*r.v11f(), target, target + step_size));
    }
}


TEST_CASE("NodeRunner: step_n", "[node_runner]") {
    std::size_t num_steps = GENERATE(1, 2, 10, 100, 1000);
    double step_size = GENERATE(0.1, 1.0);

    DummyNode node;
    auto config = TestConfig().with_step_size(DomainDuration(step_size)).with_history_capacity(0);

    NodeRunner runner{&node, config};

    auto r = runner.step_n(num_steps);
    CAPTURE(r);

    REQUIRE(r.is_successful());
    REQUIRE(r.num_steps() == num_steps);

    // Since it's the first run, we're also processing the initial time, hence the final time will be (num_steps - 1)
    auto expected_time = static_cast<double>(num_steps - 1) * step_size;
    CAPTURE(expected_time);
    REQUIRE(utils::equals(*r.v11f(), expected_time));
}

TEST_CASE("NodeRunner: step_n edge cases", "[node_runner]") {
    double start_time = 1.0;
    double step_size = 0.1;

    DummyNode node;
    auto config = TestConfig().with_step_size(DomainDuration(step_size));

    TimePoint initial_time{start_time};
    NodeRunner runner{&node, config, initial_time};


    SECTION("num steps = 0") {
        auto r = runner.step_n(0);
        CAPTURE(r);
        REQUIRE(!r.is_successful());
    }

    SECTION("Consecutiveness") {
        std::size_t num_steps = GENERATE(1, 2, 10, 100);
        auto r = runner.step_n(num_steps);

        REQUIRE(r.is_successful());
        REQUIRE(r.num_steps() == num_steps);
        auto expected_time = start_time + static_cast<double>(num_steps - 1) * step_size;

        auto last_value_first_run = *r.v11f();
        CAPTURE(last_value_first_run);

        REQUIRE(utils::equals(last_value_first_run, expected_time));

        r = runner.step_n(num_steps);
        REQUIRE(r.is_successful());
        expected_time = start_time + static_cast<double>(2 * num_steps - 1) * step_size;
        REQUIRE(utils::equals(*r.v11f(), expected_time));


        auto first_value_last_run = static_cast<double>(r.entire_output().first()->voices().first().value());
        CAPTURE(first_value_last_run);

        // Diff before start of this run and end of previous run should be equal to step size
        REQUIRE(utils::equals(first_value_last_run - last_value_first_run, step_size));
    }
}
