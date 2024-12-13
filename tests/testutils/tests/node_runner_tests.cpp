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
    DummyNode() :  NodeBase("dummy", m_ph, &m_enabled, &m_num_voices, "DummyNode") {}

    void update_time(const TimePoint& t) override {
        m_current_time = t;
    }


    Voices<Facet> process() override {
        return Voices<Facet>::singular(Facet(m_current_time.get_tick()));
    }

private:
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
    CAPTURE(target_time, step_size);

    auto config = TestConfig().with_step_size(DomainDuration(step_size)).with_history_capacity(0);
    NodeRunner runner{&node, config};

    SECTION("Stop::after") {
        for (int i = 0; i < 10; ++i) {
            auto t = target_time * (i + 1);
            auto r = runner.step_until(DomainTimePoint(t, DomainType::ticks), Stop::after);

            CAPTURE(i, t, r);
            REQUIRE(r.is_successful());

            auto v = static_cast<double>(r.last().voices().first().value());
            CAPTURE(v);
            REQUIRE(utils::in(v,  t, t + step_size));
        }
    }

    SECTION("Stop::before") {
        for (int i = 0; i < 10; ++i) {
            auto t = target_time * (i + 1);
            auto r = runner.step_until(DomainTimePoint(t, DomainType::ticks), Stop::before);

            CAPTURE(i, t, r);
            REQUIRE(r.is_successful());

            auto v = static_cast<double>(r.last().voices().first().value());
            CAPTURE(v);
            REQUIRE(utils::in(v,  t - step_size, t));
        }
    }
}


