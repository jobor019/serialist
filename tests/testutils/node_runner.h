#ifndef TESTUTILS_NODE_RUNNER_H
#define TESTUTILS_NODE_RUNNER_H
#include "generative.h"
#include "collections/vec.h"
#include "results.h"

#include "condition.h"
#include "events.h"


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



template<typename T>
class NodeRunner {

    class MeterChanges {
    public:
        struct MeterChange {
            std::size_t bar{};
            Meter meter;
        };

        void schedule(std::size_t bar, const Meter& meter) {
            // If a meter change already exists at the given bar, override it
            m_meter_changes.remove([bar](const MeterChange& c) { return c.bar == bar; });
            m_meter_changes.insert_sorted({bar, meter});
        }

        std::optional<Meter> peek(const TimePoint& t) {
            return peek(static_cast<std::size_t>(std::floor(t.get_bar())));
        }

        std::optional<Meter> peek(std::size_t bar) const {
            if (auto index = m_meter_changes.index([bar](const MeterChange& c) { return c.bar == bar; })) {
                return m_meter_changes.at(index);
            }
            return std::nullopt;
        }

        std::optional<Meter> pop(std::size_t bar) {
            return m_meter_changes.pop_value([bar](const MeterChange& c) { return c.bar == bar; });
        }

        bool empty() const { return m_meter_changes.empty(); }

    private:
        Vec<MeterChange> m_meter_changes;
    };

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


    // ==============================================================================================
    // Stepping
    // ==============================================================================================

    RunResult<T> step_until(const DomainTimePoint& t
                            , Anchor anchor
                            , const std::optional<TestConfig>& config = std::nullopt) noexcept {
        try {
            return step_internal(conditional_time(t, anchor), config.value_or(m_config), t.get_type());
        } catch (test_error& e) {
            return RunResult<T>::failure(e.what(), m_current_time, 0, t.get_type());
        }
    }


    RunResult<T> step_until(std::unique_ptr<GenericCondition<T>> stop_condition
                            , std::optional<TestConfig> config = std::nullopt) noexcept {
        config = config.value_or(m_config);
        try {
            return step_internal(conditional_output(std::move(stop_condition), true), *config, config->domain_type());
        } catch (test_error& e) {
            return RunResult<T>::failure(e.what(), m_current_time, 0, config->domain_type());
        }
    }


    RunResult<T> step_while(std::unique_ptr<GenericCondition<T>> loop_condition
                            , std::optional<TestConfig> config = std::nullopt) noexcept {
        config = config.value_or(m_config);
        try {
            return step_internal(conditional_output(std::move(loop_condition), true)
                                 , *config, config->domain_type());
        } catch (test_error& e) {
            return RunResult<T>::failure(e.what(), m_current_time, 0, config->domain_type());
        }
    }


    RunResult<T> step_n(std::size_t num_steps, std::optional<TestConfig> config = std::nullopt) noexcept {
        config = config.value_or(m_config);
        try {
            return step_internal(conditional_n(num_steps), *config, config->domain_type());
        } catch (test_error& e) {
            return RunResult<T>::failure(e.what(), m_current_time, 0, config->domain_type());
        }
    }


    RunResult<T> step(const std::optional<TestConfig>& config = std::nullopt) {
        return step_n(1, config);
    }


    // ==============================================================================================
    // Scheduling
    // ==============================================================================================

    void schedule_event(std::unique_ptr<NodeRunnerEvent<T>> event) {
        if (event->is_post_condition()) {
            m_post_condition_events.append(std::move(event));
        } else {
            m_pre_condition_events.append(std::move(event));
        }
    }


    template<typename OutputType, typename StoredType = OutputType>
    void schedule_parameter_change(Variable<OutputType, StoredType>& variable
                                   , const StoredType& value
                                   , RunnerCondition<T>&& trigger_condition) {
        schedule_event(std::make_unique<VariableChangeEvent<T, OutputType, StoredType>>(
            variable
            , value
            , std::move(trigger_condition)
        ));
    }


    template<typename OutputType, typename StoredType = OutputType, typename VoicesLike>
    void schedule_parameter_change(Sequence<OutputType, StoredType>& sequence
                                   , const VoicesLike& value
                                   , RunnerCondition<T>&& trigger_condition) {
        schedule_event(std::make_unique<SequenceChangeEvent<T, OutputType, StoredType>>(
            sequence
            , value
            , std::move(trigger_condition)
        ));
    }


    void schedule_meter_change(const Meter& new_meter, std::optional<std::size_t> bar_number) {
        if (!bar_number) {
            bar_number = m_current_time.next_bar();
        }

        if (m_current_time.get_bar() > static_cast<double>(*bar_number)) {
            throw test_error("Cannot schedule meter change in the past");
        }

        m_scheduled_meter_changes.schedule(*bar_number, new_meter);
    }


    RunnerCondition<T> conditional_n(std::size_t num_steps) const {
        if (num_steps == 0) {
            throw test_error("Step size must be > 0");
        }
        return RunnerCondition<T>::from_target_index(m_current_step_index + num_steps);
    }


    RunnerCondition<T> conditional_now() const {
        return RunnerCondition<T>::from_target_index(0);
    }


    RunnerCondition<T> conditional_time(const DomainTimePoint& t, Anchor anchor) const {
        if (t <= m_current_time) {
            throw test_error("Cannot step back in time");
        }
        return RunnerCondition<T>::from_time_point(t, anchor);
    }


    RunnerCondition<T> conditional_output(std::unique_ptr<GenericCondition<T>> output_condition
                                          , bool compare_true = true) const {
        return RunnerCondition<T>::from_generic_condition(std::move(output_condition), compare_true);
    }


    void clear_scheduled_events() {
        for (auto& node : m_pre_condition_events) {
            node->on_clear();
        }

        for (auto& node : m_post_condition_events) {
            node->on_clear();
        }

        m_pre_condition_events.clear();
        m_post_condition_events.clear();
    }


    bool has_scheduled_events() const {
        return !m_pre_condition_events.empty() || !m_post_condition_events.empty();
    }

    const TestConfig& get_config() const { return m_config; }
    const TimePoint& get_time() const { return m_current_time; }
    std::size_t get_step_index() const { return m_current_step_index; }

private:
    void check_runner_validity() {
        if (!m_output_node) {
            throw test_error("Output node not set");
        }
    }


    bool is_first_step() const { return m_current_step_index == 0; }


    /**
     *  @throws test_error if invalid values / configurations are provided.
     *  @note   If intermediate steps fails, will not throw errors but rather return `RunResult<T>::failure`
     */
    RunResult<T> step_internal(RunnerCondition<T>&& stop_condition, const TestConfig& config,
                               DomainType domain_type) {
        check_runner_validity();

        const auto& step_size = config.step_size;

        auto t_prev = m_current_time;
        auto t = m_current_time;
        if (is_first_step())
            increment_time_point(t, step_size);


        auto t_next = t;
        increment_time_point(t_next, step_size);

        auto i = m_current_step_index + 1;

        auto predicted_num_steps = stop_condition.predict_num_steps(t, m_current_step_index, step_size);
        auto step_results = predicted_num_steps
                                ? Vec<StepResult<T>>::allocated(*predicted_num_steps)
                                : Vec<StepResult<T>>();

        try {
            bool done = stop_condition.matches(m_current_step_index, t_prev, t, step_results);

            while (!done) {
                update_node_time(t);

                // Process events that should affect the current step and that don't depend on output from current step
                process_events(m_pre_condition_events, i, t, t_next, step_results);
                step_results.append(process_step(i - m_current_step_index, t, domain_type));

                if (!step_results.last()->is_successful()) {
                    break;
                }

                // Process events that depend on output from the current step
                process_events(m_post_condition_events, i, t, t_next, step_results);

                done = stop_condition.matches(i, t, t_next, step_results);

                t_prev = t;
                t = t_next;
                increment_time_point(t_next);
                ++i;
            }
        } catch (const test_error& e) {
            // Note: process_step already handles all exceptions (including test_error),
            //       the only errors caught here are the ones thrown by the loop_condition itself
            step_results.append(StepResult<T>::failure(e.what(), t, i - m_current_step_index, domain_type));
        }

        if (step_results.empty()) {
            return RunResult<T>::failure("No steps executed", m_current_time, 0, domain_type);
        }

        m_current_time = t_prev;      // last executed time
        m_current_step_index = i - 1; // last executed step

        return RunResult<T>(step_results, domain_type);
    }

    /** Update meter without incrementing the TimePoint. Will only work if
     * (a) the current TimePoint is exactly at a barline and
     * (b) a meter change is scheduled at that exact bar
     */
    void update_meter(TimePoint& t) {
        auto current_bar = static_cast<std::size_t>(std::floor(t.get_bar()));
        if (auto meter = m_scheduled_meter_changes.peek(current_bar)) {
            if (auto successful_meter_change = t.try_set_meter(meter)) {
                m_scheduled_meter_changes.pop(current_bar);
            }
        }
    }

    void increment_time_point(TimePoint& t, const DomainDuration& step_size) {
        auto next_bar = t.next_bar();
        if (auto meter = m_scheduled_meter_changes.peek(next_bar)) {
            if (auto successful_meter_change = t.try_set_meter(meter)) {
                m_scheduled_meter_changes.pop(next_bar);
            }
        }
    }


    /** @throws test_error if delta is <= 0 */
    void update_node_time(const TimePoint& current_time) {
        for (auto& g : m_generatives) {
            g->update_time(current_time);
        }
    }


    void process_events(Vec<std::unique_ptr<NodeRunnerEvent<T>>>& events
                        , std::size_t step_index, const TimePoint& t, const TimePoint& t_next
                        , const Vec<StepResult<T>>& step_results) {
        Vec<std::size_t> indices_to_remove;

        for (std::size_t i = 0; i < events.size(); ++i) {
            if (events[i]->process(step_index, t, t_next, step_results)) {
                indices_to_remove.append(i);
            }
        }
        events.erase(indices_to_remove);
    }


    StepResult<T> process_step(std::size_t step_index, const TimePoint& current_time, const DomainType& t) noexcept {
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
    std::size_t m_current_step_index = 0;

    Vec<Generative*> m_generatives;
    Node<T>* m_output_node = nullptr;

    DomainDuration m_default_step_size;

    Vec<std::unique_ptr<NodeRunnerEvent<T>>> m_pre_condition_events;
    Vec<std::unique_ptr<NodeRunnerEvent<T>>> m_post_condition_events;

    MeterChanges m_scheduled_meter_changes;
};
}


#endif // TESTUTILS_NODE_RUNNER_H
