#include <catch2/generators/catch_generators.hpp>

#include "core/policies/policies.h"
#include "node_runner.h"
#include "algo/facet.h"
#include "conditions/c11.h"
#include "generatives/sequence.h"
#include "generatives/variable.h"
#include "generatives/stereotypes/base_stereotypes.h"
using namespace serialist;
using namespace serialist::test;

/** Dummy node that returns current time as a facet (independently of number of voices) */
class DummyNode : public NodeBase<Facet> {
public:
    explicit DummyNode(DomainType domain_type = DomainType::ticks)
        : NodeBase("dummy", m_ph, &m_enabled, &m_num_voices, "DummyNode")
          , m_domain_type(domain_type) {}


    void update_time(const TimePoint& t) override {
        m_current_time = t;
    }


    Voices<Facet> process() override {
        double add = m_add_seq.process().first_or(0.0);
        double mul = m_mul_var.process().first_or(1.0);
        return Voices<Facet>::singular(Facet(mul * m_current_time.get(m_domain_type)) + add);
    }


    void set_domain_type(DomainType domain_type) { m_domain_type = domain_type; }

    Variable<Facet, double>& mul_var() { return m_mul_var; }
    Sequence<Facet, double>& add_seq() { return m_add_seq; }

private:
    DomainType m_domain_type;

    ParameterHandler m_ph = ParameterHandler();

    // Dummy Variable / Sequence for testing
    Variable<Facet, double> m_mul_var{"mul", m_ph, 1.0};
    Sequence<Facet, double> m_add_seq{"add", m_ph, 0.0};

    // Unused but required by NodeBase
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

TEST_CASE("NodeRunner: step_until (time)", "[node_runner]") {
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
            auto r = runner.step_until(DomainTimePoint(t, domain_type), Anchor::after);

            CAPTURE(i, t, r);
            REQUIRE(r.is_successful());

            REQUIRE(utils::in(*r.v11f(), t, t + step_size));
        }
    }

    SECTION("Stop::before") {
        for (int i = 0; i < 10; ++i) {
            auto t = target_time * (i + 1);
            auto r = runner.step_until(DomainTimePoint(t, domain_type), Anchor::before);

            CAPTURE(i, t, r);
            REQUIRE(r.is_successful());

            REQUIRE(utils::in(*r.v11f(), t - step_size, t));
        }
    }
}


TEST_CASE("NodeRunner: step_until (time) edge cases", "[node_runner]") {
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
        auto r = runner.step_until(DomainTimePoint(0.0, DomainType::ticks), Anchor::before);
        CAPTURE(r);
        REQUIRE(!r.is_successful());
    }

    SECTION("Run to current time (Before)") {
        auto r = runner.step_until(DomainTimePoint(start_time, DomainType::ticks), Anchor::before);
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
        auto r = runner.step_until(DomainTimePoint(start_time + step_size, DomainType::ticks), Anchor::after);
        CAPTURE(r);
        REQUIRE(r.is_successful());
        REQUIRE(r.last().voices().first().value() >= start_time + step_size);
    }

    SECTION("Run to current time + step_size (Before)") {
        // On first run, this should be possible, as it will generate a single step at current time without incrementing
        auto r = runner.step_until(DomainTimePoint(start_time + step_size, DomainType::ticks), Anchor::before);
        REQUIRE(r.is_successful());
        REQUIRE(r.num_steps() == 1);
    }

    SECTION("Run to given time twice (Before)") {
        auto r = runner.step_until(DomainTimePoint(2.0, DomainType::ticks), Anchor::before);
        REQUIRE(r.is_successful());
        r = runner.step_until(DomainTimePoint(2.0, DomainType::ticks), Anchor::before);
        CAPTURE(r);
        REQUIRE(!r.is_successful());
    }

    SECTION("Run to current time + step_size/2 (After)") {
        // Consume first step
        auto r = runner.step_until(DomainTimePoint(2.0, DomainType::ticks), Anchor::after);
        REQUIRE(r.is_successful());
        r = runner.step_until(DomainTimePoint(2.0 + step_size / 2, DomainType::ticks), Anchor::after);
        CAPTURE(r);
        REQUIRE(r.is_successful());
    }

    SECTION("Consecutive steps (before)") {
        auto target = GENERATE(2.0, 3.75, 10.0);
        auto r = runner.step_until(DomainTimePoint(target, DomainType::ticks), Anchor::before);
        REQUIRE(r.is_successful());
        REQUIRE(utils::in(*r.v11f(), target - step_size, target));

        r = runner.step();
        CAPTURE(r);
        REQUIRE(r.is_successful());
        REQUIRE(utils::in(*r.v11f(), target, target + step_size));
    }
}


TEST_CASE("NodeRunner: step_until (condition)", "[node_runner]") {
    DummyNode node;
    auto step_size = 0.1;
    auto config = TestConfig().with_step_size(DomainDuration(step_size));

    NodeRunner runner{&node, config};

    auto target = GENERATE(2.0, 3.75, 10.0);


    // auto r = runner.step_until(std::make_unique<CompareValue<Facet>>([target](const Facet& f) {
    //     return static_cast<double>(f) >= target;
    // }));

    // auto r = runner.step_until(c11::ge(Facet(target)));
    auto r = runner.step_until(c11::gef(target));
    REQUIRE(r.is_successful());
    REQUIRE(utils::in(*r.v11f(), target, target + config.step_size.get_value()));
    REQUIRE(!r.history().empty());
}


TEST_CASE("NodeRunner: step_while (condition)", "[node_runner]") {
    DummyNode node;
    auto step_size = 0.1;
    auto config = TestConfig().with_step_size(DomainDuration(step_size));

    NodeRunner runner{&node, config};

    SECTION("Positive cases") {
        auto target = GENERATE(2.0, 3.75, 10.0);

        auto r = runner.step_while(c11::lef(target));
        REQUIRE(r.is_successful());

        REQUIRE(!r.history().empty());
        for (const auto& step : r.history()) {
            REQUIRE(*step.v11f() < target);
        }

        REQUIRE(utils::in(*r.v11f(), target, target + config.step_size.get_value()));
        REQUIRE(!r.history().empty());
    }

    SECTION("Negative cases") {
        auto r = runner.step_while(c11::gef(100.0));
        REQUIRE(r.is_successful());
        REQUIRE(r.num_steps() == 1);
        REQUIRE(*r.v11f() < 100.0);
    }
}


TEST_CASE("NodeRunner: step_n", "[node_runner]") {
    // std::size_t num_steps = GENERATE(1, 2, 10, 100, 1000);
    std::size_t num_steps = 1;
    // double step_size = GENERATE(0.1, 1.0);
    double step_size = GENERATE(0.1);

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

    SECTION("Run Consecutiveness") {
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


TEST_CASE("NodeRunner: schedule variable change (num steps)", "[node_runner]") {
    double step_size = 0.1;
    DummyNode node;
    auto config = TestConfig().with_step_size(DomainDuration(step_size));

    NodeRunner runner{&node, config};

    runner.schedule_parameter_change(node.mul_var(), 2.0, runner.conditional_n(10));

    // Multiple steps up to parameter change
    auto r = runner.step_n(9);
    REQUIRE(r.is_successful());
    REQUIRE(utils::equals(*r.v11f(),step_size * 8.0));
    REQUIRE(runner.has_scheduled_events());

    // Single step to change parameter
    r = runner.step();
    REQUIRE(r.is_successful());
    REQUIRE(utils::equals(*r.v11f(),step_size * 9.0 * 2.0));
    REQUIRE(!runner.has_scheduled_events());

    runner.schedule_parameter_change(node.mul_var(), 0.5, runner.conditional_now());

    // Single step to change parameter immediately
    r = runner.step();
    REQUIRE(r.is_successful());
    REQUIRE(utils::equals(*r.v11f(),step_size * 10.0 * 0.5));
    REQUIRE(!runner.has_scheduled_events());
}


TEST_CASE("NodeRunner: schedule sequence change (num steps)", "[node_runner]") {
    double step_size = 0.1;
    DummyNode node;
    NodeRunner runner{&node, TestConfig().with_step_size(DomainDuration(step_size))};

    auto cond = runner.conditional_n(5);

    SECTION("const T&") {
        runner.schedule_parameter_change(node.add_seq(), 10.0, std::move(cond));
        auto r = runner.step_n(5);
        REQUIRE(r.is_successful());
        REQUIRE(utils::equals(*r.v11f(), step_size * 4.0 + 10.0));
    }

    SECTION("const Vec<T>&") {
        runner.schedule_parameter_change(node.add_seq(), Vec{5.0, 6.0, 7.0}, std::move(cond));
        auto r = runner.step_n(5);
        REQUIRE(r.is_successful());
        REQUIRE(utils::equals(*r.v11f(), step_size * 4.0 + 5.0)); // t + first value in sequence
    }

    SECTION("const Voices<T>&") {
        runner.schedule_parameter_change(node.add_seq(), Voices<double>::transposed({2.0, 3.0, 4.0}),
                                         std::move(cond));
        auto r = runner.step_n(5);
        REQUIRE(r.is_successful());
        REQUIRE(utils::equals(*r.v11f(), step_size * 4.0 + 2.0)); // t + first value in sequence
    }
}


TEST_CASE("NodeRunner: schedule variable change (time)", "[node_runner]") {
    DummyNode node;

    double step_size = 0.01;
    NodeRunner runner{&node, TestConfig().with_step_size(DomainDuration(step_size))};

    auto target_time = DomainTimePoint(10.0, DomainType::ticks);


    SECTION("Before") {
        auto anchor = Anchor::before;
        auto cond = runner.conditional_time(target_time, anchor);
        runner.schedule_parameter_change(node.mul_var(), 2.0, std::move(cond));

        auto r = runner.step_until(target_time, anchor);
        REQUIRE(r.is_successful());
        REQUIRE(utils::in(*r.history_subset().v11f(), 10.0 - 2 * step_size, 10.0 - step_size));
        REQUIRE(utils::in(*r.v11f(), (10.0 - step_size) * 2, 10.0 * 2));
    }

    SECTION("After") {
        auto anchor = Anchor::after;
        auto cond = runner.conditional_time(target_time, anchor);
        runner.schedule_parameter_change(node.mul_var(), 2.0, std::move(cond));

        auto r = runner.step_until(target_time, anchor);
        REQUIRE(r.is_successful());
        REQUIRE(utils::in(*r.history_subset().v11f(), 10.0 - step_size, 10.0));
        REQUIRE(utils::in(*r.v11f(), 10.0 * 2, (10.0 + step_size) * 2));
    }
}


TEST_CASE("NodeRunner: schedule variable change (output condition)", "[node_runner]") {
    DummyNode node;

    double step_size = 0.01;
    NodeRunner runner{&node, TestConfig().with_step_size(DomainDuration(step_size))};

    auto cond = runner.conditional_output(c11::gef(2.0));
    runner.schedule_parameter_change(node.mul_var(), 2.0, std::move(cond));

    auto r = runner.step_until(DomainTimePoint(2.0, DomainType::ticks), Anchor::after);
    // Since conditional outputs are not evaluated until the end of the step, we expect no change to the output here
    REQUIRE(r.is_successful());
    REQUIRE(utils::in(*r.v11f(), 2.0, 2.0 + step_size));
    // However, at the next step, we expect the change to have been applied
    r = runner.step();
    REQUIRE(r.is_successful());
    REQUIRE(utils::in(*r.v11f(), 2.0 * 2, (2.0 + 2 * step_size) * 2));
}

TEST_CASE("NodeRunner: schedule meter change", "[node_runner]") {
    DummyNode node;
    NodeRunner runner{&node};

    SECTION("Schedule at current time (at start of bar)") {
        // Schedule as soon as possible = at the start of the current bar if bar % 1.0 ≈ 0.0
        runner.schedule_meter_change(Meter{2, 8}, std::nullopt);
        auto r = runner.step();
        REQUIRE(r.last().time().get_meter() == Meter{2, 8});
    }

    SECTION("Schedule as soon as possible (in the middle of bar)") {
        runner.step_until(DomainTimePoint::bars(0.5), Anchor::after);
        runner.schedule_meter_change(Meter{2, 8}, std::nullopt);
        auto r = runner.step_until(DomainTimePoint::bars(1.0), Anchor::before);
        REQUIRE(r.last().time().get_meter() == Meter{4, 4}); // initial meter
        r = runner.step();
        REQUIRE(r.last().time().get_meter() == Meter{2, 8});
    }

    SECTION("Schedule at future bar") {
        runner.schedule_meter_change(Meter{2, 8}, 10);

        auto r = runner.step_until(DomainTimePoint(10.0, DomainType::bars), Anchor::before);
        REQUIRE(r.last().time().get_meter() == Meter{4, 4}); // initial meter
        r = runner.step();
        REQUIRE(r.last().time().get_meter() == Meter{2, 8});
    }

    SECTION("Multiple scheduled meter changes") {
        runner.schedule_meter_change(Meter{1, 8}, 2);
        runner.schedule_meter_change(Meter{2, 8}, 4);

        auto r = runner.step_until(DomainTimePoint(2.0, DomainType::bars), Anchor::before);
        REQUIRE(r.last().time().get_meter() == Meter{4, 4}); // initial meter

        r = runner.step();
        REQUIRE(r.last().time().get_meter() == Meter{1, 8});
        r = runner.step_until(DomainTimePoint(4.0, DomainType::bars), Anchor::before);
        REQUIRE(r.last().time().get_meter() == Meter{1, 8});

        r = runner.step();
        REQUIRE(r.last().time().get_meter() == Meter{2, 8});
    }

    SECTION("Schedule at elapsed time") {
        auto m = Meter{4, 4};
        runner.step_until(DomainTimePoint::bars(0.5), Anchor::after);
        REQUIRE_THROWS(runner.schedule_meter_change(m, 0));
        runner.step_until(DomainTimePoint::bars(4), Anchor::after);
        REQUIRE_THROWS(runner.schedule_meter_change(m, 0));
        REQUIRE_THROWS(runner.schedule_meter_change(m, 3));
    }
}


// ==============================================================================================

TEST_CASE("RunResult: history_subset, last_subset, unpack", "[node_runner]") {
    DummyNode node;
    NodeRunner runner{&node};

    SECTION("Positive cases") {
        auto r = runner.step_n(10);
        REQUIRE(r.is_successful());
        REQUIRE(r.num_steps() == 10);
        REQUIRE(r.v11f().has_value());
        REQUIRE(r.last().v11f().has_value());
        REQUIRE(r.last_subset().v11f().has_value());
        REQUIRE(r.history_subset().num_steps() == 9);

        auto [hist, last] = r.unpack();
        REQUIRE(hist.num_steps() == 9);
        REQUIRE(last.v11f().has_value());
        REQUIRE(last.v11f().value() == *r.v11f());
    }

    SECTION("Negative cases") {
        auto r = runner.step();
        REQUIRE(r.is_successful());
        REQUIRE(r.num_steps() == 1);
        REQUIRE(r.v11f().has_value());
        REQUIRE(r.last().v11f().has_value());
        REQUIRE(r.last_subset().v11f().has_value());

        // No history
        REQUIRE_THROWS_AS(r.history_subset(), test_error);
        REQUIRE_THROWS_AS(r.unpack(), test_error);
    }
}