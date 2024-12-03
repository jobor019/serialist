#ifndef NODE_RUNNER_H
#define NODE_RUNNER_H
#include "generative.h"
#include "collections/vec.h"
#include "runner_results.h"
#include <catch2/catch_test_macros.hpp>

using namespace serialist;

enum class StepRounding {
    exact_stop_before, exact_stop_after, round_all, round_first, round_last
};


class Steps {
public:
    static Steps from_duration(const DomainDuration& duration, const DomainDuration& step_size, StepRounding rounding) {
        assert(step_size.get_value() > 0);
        assert(duration.get_value() > step_size.get_value());
        assert(duration.get_type() == step_size.get_type());

        double r = duration.get_value();
        double q = duration.get_value() / step_size.get_value();
        DomainType t = duration.get_type();

        if (rounding == StepRounding::exact_stop_before) {
            return Steps(step_size, rounding, static_cast<std::size_t>(std::floor(q)));
        }

        if (rounding == StepRounding::exact_stop_after) {
            return Steps(step_size, rounding, static_cast<std::size_t>(std::ceil(q)));
        }

        if (rounding == StepRounding::round_all) {
            double n = std::round(q);
            return Steps(DomainDuration(r / n, t), rounding, static_cast<std::size_t>(n));
        }

        if (rounding == StepRounding::round_first || rounding == StepRounding::round_last) {
            double n = std::ceil(q);
            auto num_steps = static_cast<std::size_t>(n);
            auto fractional_step = step_size * utils::modulo(q, 1.0);

            auto first = step_size;
            auto mid = step_size;
            auto last = step_size;

            if (!utils::equals(fractional_step.get_value(), 0.0)) {
                if (rounding == StepRounding::round_first) {
                    first = fractional_step;
                } else {
                    // lazy assertion: implementation doesn't handle this edge case. If there are fewer than 3 steps,
                    //                 the last value won't be used and the duration will be incorrect
                    assert(num_steps >= 3);
                    last = fractional_step;
                }
            }

            return Steps(first, mid, last, rounding, num_steps);
        }

        throw std::runtime_error("Invalid rounding value");
    }


    static Steps from_num_steps(std::size_t num_steps, const DomainDuration& step_size) {
        // Note: rounding is not relevant in this case
        return Steps(step_size, StepRounding::round_all, num_steps);
    }


    const DomainDuration& first() const {
        return m_first_delta;
    }


    const DomainDuration& default_delta() const {
        return m_delta;
    }


    const DomainDuration& last() const {
        return m_last_delta;
    }


    std::size_t num_steps() const {
        return m_num_steps;
    }


    DomainType domain() const {
        return m_delta.get_type();
    }


private:
    Steps(const DomainDuration& uniform_step_size, StepRounding rounding, std::size_t num_steps)
            : Steps(uniform_step_size, uniform_step_size, uniform_step_size, rounding, num_steps) { }

    Steps(const DomainDuration& first_delta
        , const DomainDuration&  delta
        , const DomainDuration& last_delta
        , StepRounding rounding, std::size_t num_steps)
            :  m_first_delta(first_delta)
    , m_delta(delta)
    , m_last_delta(last_delta)
    , m_rounding(rounding)
    , m_num_steps(num_steps) {
        assert(num_steps > 0);
        assert(m_first_delta.get_value() > 0);
        assert(m_delta.get_value() > 0);
        assert(m_last_delta.get_value() >= 0); // Last delta can be exactly 0 if no rounding occurs
    }


    void set_all(const DomainDuration& delta) {
        m_first_delta = delta;
        m_delta = delta;
        m_last_delta = delta;
    }


    DomainDuration m_first_delta{0.0};
    DomainDuration m_delta{0.0};
    DomainDuration m_last_delta{0.0};

    StepRounding m_rounding;

    std::size_t m_num_steps = 0;
};


// ==============================================================================================


// template<typename T>
class TestConfig {
public:
    const inline static auto DEFAULT_STEP_SIZE = DomainDuration(0.01, DomainType::ticks);
    constexpr static auto DEFAULT_DOMAIN_TYPE = DomainType::ticks;
    constexpr static std::optional<std::size_t> DEFAULT_HISTORY_CAPACITY = std::nullopt;
    constexpr static auto DEFAULT_STEP_ROUNDING = StepRounding::round_last;


    TestConfig() = default;


    TestConfig cloned() const {
        return *this;
    }


    /** Note: use std::nullopt for infinite capacity */
    TestConfig& with_history_capacity(std::optional<std::size_t> capacity) {
        // TODO: There may be use cases in the future where we iterate over a lot of steps, and having a capacity
        //       then is a strict requirement, but for now, we'll only support either infinite capacity or 0 capacity
        assert(capacity == std::nullopt || capacity == 0);
        history_capacity = capacity;
        return *this;
    }


    TestConfig& with_step_rounding(StepRounding rounding) {
        step_rounding = rounding;
        return *this;
    }

    TestConfig& with_step_size(const DomainDuration& size) {
        step_size = size;
        return *this;
    }


    // TestConfig& add_history_assertion(StepAssertion<T> assertion) {
    //     assertions.append(assertion);
    //     return *this;
    // }


    // Vec<StepAssertion<T> > assertions;
    DomainDuration step_size = DEFAULT_STEP_SIZE;
    StepRounding step_rounding = DEFAULT_STEP_ROUNDING;
    std::optional<std::size_t> history_capacity = DEFAULT_HISTORY_CAPACITY;
};


// ==============================================================================================

template<typename T>
class NodeRunner {
public:
    explicit NodeRunner(Node<T>* output_node = nullptr
                              , const TestConfig& config = TestConfig()
                              , const TimePoint& initial_time = TimePoint{})
        : m_config(config)
          , m_current_time(initial_time) {
        if (output_node) {
            set_output_node(*output_node);
        }
    }


    NodeRunner& add_generative(Generative& g) {
        if (!m_generatives.contains(&g)) {
            m_generatives.append(&g);
        }
        return *this;
    }


    // GenerativeRunner& add_persistent_trigger_node(Node<Trigger>& node) {
    //     if (!m_persistent_trigger_nodes.contains(&node)) {
    //         m_persistent_trigger_nodes.append(&node);
    //     }
    //     return *this;
    // }


    NodeRunner& set_output_node(Node<T>& node) {
        auto* generative = dynamic_cast<Generative*>(&node);


        if (!m_generatives.contains(generative)) {
            m_generatives.append(generative);
        }

        m_output_node = &node;

        return *this;
    }


    RunResult<T> step_until(const DomainTimePoint& t
                            , std::optional<TestConfig> config = std::nullopt) {
        assert(t > m_current_time);
        config = config.value_or(m_config);

        auto duration = DomainDuration::distance(m_current_time, t);
        auto steps = Steps::from_duration(duration, config->step_size, config->step_rounding);

        return step_internal(steps, *config);
    }

    RunResult<T> step_n(std::size_t num_steps, std::optional<TestConfig> config = std::nullopt) {
        assert(num_steps > 0); // This is most likely an error in the caller

        config = config.value_or(m_config);

        auto steps = Steps::from_num_steps(num_steps, config->step_size);

        return step_internal(steps, *config);
    }


    // TODO
    // RunResult<T> step_for(const DomainDuration& t
    //                       , StepOutput<T> previous_step_handling = StepOutput<T>::output()
    //                       , std::optional<DomainDuration> step_size = std::nullopt) {
    //     throw std::runtime_error("Not implemented");
    // }
    //
    //
    // // TODO: Pass lambda
    // RunResult<T> step_while() { throw std::runtime_error("Not implemented"); };
    //
    // RunResult<T> step_n(std::size_t n) { throw std::runtime_error("Not implemented"); }
    //
    // RunResult<T> discontinuity(const DomainTimePoint& new_time) { throw std::runtime_error("Not implemented"); }
    //
    //
    // template<typename NodeValueType>
    // GenerativeRunner& schedule_parameter_change(Node<NodeValueType>& node
    //                                , const NodeValueType& new_value
    //                                , const DomainTimePoint& time) {
    //     // TODO: Add node as output node if it's not already in m_generatives
    //     throw std::runtime_error("Not implemented");
    // }
    //
    //
    // template<typename NodeValueType>
    // GenerativeRunner& schedule_parameter_ramp(Node<NodeValueType>& node
    //                              , const NodeValueType& start_value
    //                              , const NodeValueType& end_value
    //                              , const DomainTimePoint& end_time
    //                              , const std::optional<DomainTimePoint>& start_time = std::nullopt) {
    //     // TODO: Add node as output node if it's not already in m_generatives
    //     throw std::runtime_error("Not implemented");
    // }
    //
    //
    // GenerativeRunner& set_tempo() { throw std::runtime_error("Not implemented"); }
    //
    // GenerativeRunner& schedule_tempo_change() { throw std::runtime_error("Not implemented"); }
    //
    // GenerativeRunner& schedule_tempo_ramp() { throw std::runtime_error("Not implemented"); }
    //
    // GenerativeRunner& schedule_meter_change() { throw std::runtime_error("Not implemented"); }

    const TestConfig& get_config() const {
        return m_config;
    }

private:
    void check_runner_validity() {
        if (!m_output_node) {
            throw std::runtime_error("Output node not set");
        }
    }

    RunResult<T> step_internal(const Steps& steps, const TestConfig& config) {
        check_runner_validity();

        auto outputs = Vec<StepResult<T> >::allocated(steps.num_steps() - 1);

        for (int i = 0; i < steps.num_steps() - 1; ++i) {
            if (i == 0) {
                update_time(steps.first());
            } else {
                update_time(steps.default_delta());
            }

            auto output = process_step(false, i, steps.domain());

            if (!output.success) {
                return RunResult<T>(output, outputs, false, steps.domain());
            }

            if (!config.history_capacity || *(config.history_capacity) > 0) {
                // TODO: Handle exact size with circular buffer
                outputs.append(output);
            }
        }

        update_time(steps.last());
        auto output = process_step(true, steps.num_steps() - 1, steps.domain());

        return RunResult<T>(output, outputs, output.success, steps.domain());
    }



    void update_time(const DomainDuration& delta) {
        assert(delta.get_value() > 0.0);

        m_current_time += delta;

        for (auto& g: m_generatives) {
            g->update_time(m_current_time);
        }
    }


    StepResult<T> process_step(bool is_last_step
                               , std::size_t step_index
                               // , const Vec<StepAssertion<T> >& assertions
                               , const DomainType& t) {
        // TODO: we will need to handle parameter / meter / time signature ramps/scheduled changes here too in the future!!

        // for (auto& trigger_node : m_persistent_trigger_nodes) {
        //     trigger_node.
        // }

        auto output = m_output_node->process();

        if (is_last_step) {
            return StepResult<T>(m_current_time, output, step_index, true, t);
        }

        // auto success = assertions.all([&output](const StepAssertion<T>& assertion) {
        //     return assertion.do_assert(output);
        // });

        return StepResult<T>(m_current_time, output, step_index, true, t);
    }


    TestConfig m_config;

    TimePoint m_current_time;

    Vec<Generative*> m_generatives;
    // Vec<Sequence<Trigger>*> m_persistent_trigger_nodes;
    Node<T>* m_output_node = nullptr;

    DomainDuration m_default_step_size;
};

#endif //NODE_RUNNER_H
