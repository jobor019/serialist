#ifndef TESTUTILS_NODE_RUNNER_H
#define TESTUTILS_NODE_RUNNER_H
#include "generative.h"
#include "collections/vec.h"
#include "results.h"

#include "condition.h"


namespace serialist::test {

class TestConfig {
public:
    const inline static auto DEFAULT_STEP_SIZE = DomainDuration(0.01, DomainType::ticks);
    constexpr static auto DEFAULT_DOMAIN_TYPE = DomainType::ticks;
    constexpr static std::optional<std::size_t> DEFAULT_HISTORY_CAPACITY = std::nullopt;


    TestConfig() = default;


    TestConfig cloned() const {
        return *this;
    }


    /**
     * @throws test_error if `capacity` is not 0 or std::nullopt
     * @note: use std::nullopt for infinite capacity
     */
    TestConfig& with_history_capacity(std::optional<std::size_t> capacity) {
        // TODO: There may be use cases in the future where we iterate over a lot of steps, and having a capacity
        //       then is a strict requirement, but for now, we'll only support either infinite capacity or 0 capacity
        if (capacity.has_value() && capacity.value() != 0)
            throw test_error("History capacity must be 0 or std::nullopt");

        history_capacity = capacity;
        return *this;
    }


    TestConfig& with_step_size(const DomainDuration& size) {
        if (size.get_value() <= 0) {
            throw test_error("Step size must be > 0");
        }

        step_size = size;
        return *this;
    }


    DomainType domain_type() const { return step_size.get_type(); }


    DomainDuration step_size = DEFAULT_STEP_SIZE;
    std::optional<std::size_t> history_capacity = DEFAULT_HISTORY_CAPACITY;
};


// ==============================================================================================

enum class Stop {
    before, after
};


template<typename T>
class LoopCondition {
public:
    struct NumSteps {
        std::size_t index;
    };
    struct StopAfter {
        DomainTimePoint time;
    };
    struct StopBefore {
        DomainTimePoint time;
    };
    struct CompareTrue {
        std::unique_ptr<GenericCondition<T> > condition;
    };
    struct CompareFalse {
        std::unique_ptr<GenericCondition<T> > condition;
    };

    using Condition = std::variant<NumSteps, StopAfter, StopBefore, CompareTrue, CompareFalse>;

    explicit LoopCondition(Condition condition) : m_condition(std::move(condition)) {}


    static LoopCondition from_stop_type(const DomainTimePoint& t, Stop stop_type) {
        if (stop_type == Stop::before) {
            return LoopCondition(StopBefore{t});
        } else {
            return LoopCondition(StopAfter{t});
        }
    }


    static LoopCondition from_num_steps(std::size_t num_steps) {
        return LoopCondition(NumSteps{num_steps});
    }


    static LoopCondition from_generic_condition(std::unique_ptr<GenericCondition<T> > c, bool compare_true) {
        if (compare_true) {
            return LoopCondition(CompareTrue{std::move(c)});
        } else {
            return LoopCondition(CompareFalse{std::move(c)});
        }
    }


    /** Naively predicts number of steps until stop condition,
     *  under the assumption that the step size is constant and meter doesn't change */
    std::optional<std::size_t> predict_num_steps(const TimePoint& current
                                                 , const DomainDuration& step_size) const {
        return std::visit([&](const auto& c) -> std::optional<std::size_t> {
            using VariantType = std::decay_t<decltype(c)>;

            if constexpr (std::is_same_v<VariantType, NumSteps>) {
                return c.index;
            } else if constexpr (std::is_same_v<VariantType, StopAfter>) {
                return steps(current, c.time, step_size) + 1;
            } else if constexpr (std::is_same_v<VariantType, StopBefore>) {
                return steps(current, c.time, step_size);
            } else {
                return std::nullopt; // Cannot predict number of steps for other conditions
            }
        }, m_condition);
    }


    /** Note: continue while condition is true */
    bool operator()(std::size_t step_index
                    , const TimePoint& t
                    , const TimePoint& t_prev
                    , const Vec<StepResult<T> >& v) const {
        return std::visit([&](const auto& cond) -> bool {
            using VariantType = std::decay_t<decltype(cond)>;
            if constexpr (std::is_same_v<VariantType, NumSteps>) {
                return step_index < cond.index;
            } else if constexpr (std::is_same_v<VariantType, StopAfter>) {
                return t_prev < cond.time;
            } else if constexpr (std::is_same_v<VariantType, StopBefore>) {
                return t < cond.time;
            } else if constexpr (std::is_same_v<VariantType, CompareTrue>) {
                return cond.condition->matches_last(v).value_or(true);
            } else if constexpr (std::is_same_v<VariantType, CompareFalse>) {
                return !cond.condition->matches_last(v).value_or(false);
            } else {
                throw test_error("Unsupported condition type");
            }
        }, m_condition);
    }


    template<typename U>
    const U& as() const { return std::get<U>(m_condition); }


    template<typename U>
    bool is() const { return std::holds_alternative<U>(m_condition); }

private:
    static std::size_t steps(const TimePoint& current, const DomainTimePoint& target, const DomainDuration& step_size) {
        auto distance = (target - current).as_type(step_size.get_type(), current.get_meter());
        return static_cast<std::size_t>(std::ceil(distance.get_value() / step_size.get_value()));
    }


    Condition m_condition;
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


    NodeRunner& set_output_node(Node<T>& node) {
        auto* generative = dynamic_cast<Generative*>(&node);


        if (!m_generatives.contains(generative)) {
            m_generatives.append(generative);
        }

        m_output_node = &node;

        return *this;
    }


    RunResult<T> step_until(const DomainTimePoint& t
                            , Stop stop_type
                            , std::optional<TestConfig> config = std::nullopt) noexcept {
        if (t <= m_current_time) {
            return RunResult<T>::failure("Cannot step back in time (requested time: " + t.to_string() + ")"
                                         , m_current_time, 0, t.get_type());
        }

        config = config.value_or(m_config);
        auto loop_condition = LoopCondition<T>::from_stop_type(t, stop_type);

        try {
            return step_internal(loop_condition, *config, t.get_type());
        } catch (test_error& e) {
            return RunResult<T>::failure(e.what(), m_current_time, 0, t.get_type());
        }
    }


    RunResult<T> step_until(std::unique_ptr<GenericCondition<T> > stop_condition
                            , std::optional<TestConfig> config = std::nullopt) noexcept {
        config = config.value_or(m_config);
        auto loop_condition = LoopCondition<T>::from_generic_condition(std::move(stop_condition), false);
        try {
            return step_internal(loop_condition, *config, config->domain_type());
        } catch (test_error& e) {
            return RunResult<T>::failure(e.what(), m_current_time, 0, config->domain_type());
        }
    }


    RunResult<T> step_while(std::unique_ptr<GenericCondition<T> > step_condition
                            , std::optional<TestConfig> config = std::nullopt) noexcept {
        config = config.value_or(m_config);
        auto loop_condition = LoopCondition<T>::from_generic_condition(std::move(step_condition), true);
        try {
            return step_internal(loop_condition, *config, config->domain_type());
        } catch (test_error& e) {
            return RunResult<T>::failure(e.what(), m_current_time, 0, config->domain_type());
        }
    }


    RunResult<T> step_n(std::size_t num_steps, std::optional<TestConfig> config = std::nullopt) noexcept {
        if (num_steps == 0) {
            return RunResult<T>::failure("Step size must be > 0", m_current_time);
        }

        config = config.value_or(m_config);
        auto loop_condition = LoopCondition<T>::from_num_steps(num_steps);

        try {
            return step_internal(loop_condition, *config, config->domain_type());
        } catch (test_error& e) {
            return RunResult<T>::failure(e.what(), m_current_time, 0, config->domain_type());
        }
    }


    RunResult<T> step(std::optional<TestConfig> config = std::nullopt) {
        return step_n(1, std::move(config));
    }


    // TODO
    //
    // TODO: If we ever implement step_for, it should always step for that duration,
    //       not step until t + duration (as meter may change)
    // RunResult<T> step_for(const DomainDuration& t
    //                       , StepOutput<T> previous_step_handling = StepOutput<T>::output()
    //                       , std::optional<DomainDuration> step_size = std::nullopt) {
    //     throw std::runtime_error("Not implemented");
    // }
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
            throw test_error("Output node not set");
        }
    }


    /**
     *  @throws test_error if invalid values / configurations are provided.
     *  @note   If intermediate steps fails, will not throw errors but rather return `RunResult<T>::failure`
     */
    RunResult<T> step_internal(const LoopCondition<T>& loop_condition, const TestConfig& config,
                               DomainType domain_type) {
        check_runner_validity();

        const auto& step_size = config.step_size;

        auto t_prev = m_current_time;
        auto t = m_is_first_step ? t_prev : t_prev.incremented(step_size);

        auto predicted_num_steps = loop_condition.predict_num_steps(t, step_size);
        auto step_results = predicted_num_steps
                                ? Vec<StepResult<T> >::allocated(*predicted_num_steps)
                                : Vec<StepResult<T> >();

        std::size_t i = 0;

        try {
            while (loop_condition(i, t, t_prev, step_results)) {
                update_node_time(t);

                step_results.append(process_step(i, t, domain_type));

                if (!step_results.last()->is_successful()) {
                    break;
                }

                t_prev = t;
                t += step_size;
                ++i;
            }
        } catch (const test_error& e) {
            // Note: process_step already handles all exceptions (including test_error),
            //       the only errors caught here are the ones thrown by the loop_condition itself
            step_results.append(StepResult<T>::failure(e.what(), t, i, domain_type));
        }

        if (step_results.empty()) {
            return RunResult<T>::failure("No steps executed", m_current_time, 0, domain_type);
        }

        m_is_first_step = false;
        m_current_time = t_prev; // last executed time

        return RunResult<T>(step_results, domain_type);
    }


    /** @throws test_error if delta is <= 0 */
    void update_node_time(const TimePoint& current_time) {
        for (auto& g: m_generatives) {
            g->update_time(current_time);
        }
    }


    StepResult<T> process_step(std::size_t step_index, const TimePoint& current_time, const DomainType& t) noexcept {
        // TODO: we will need to handle parameter / meter / time signature ramps/scheduled changes here too in the future!!
        try {
            auto output = m_output_node->process();
            return StepResult<T>::success(output, current_time, step_index, t);
        } catch (const std::exception& e) {
            return StepResult<T>::failure(e.what(), current_time, step_index, t);
        } catch (...) {
            return StepResult<T>::failure("Unknown exception", current_time, step_index, t);
        }
    }


    TestConfig m_config;

    TimePoint m_current_time;

    Vec<Generative*> m_generatives;
    Node<T>* m_output_node = nullptr;

    DomainDuration m_default_step_size;

    bool m_is_first_step = true;
};
}

#endif // TESTUTILS_NODE_RUNNER_H
