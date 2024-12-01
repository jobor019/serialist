#ifndef GENERATIVE_RUNNER_H
#define GENERATIVE_RUNNER_H
#include "generative.h"
#include "collections/vec.h"
#include "runner_results.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace serialist;

enum class StepRounding {
    exact_stop_before, exact_stop_after, round_all, round_first, round_last,
};


class Steps {
public:
    // class Iterator {
    // public:
    //     using iterator_category = std::input_iterator_tag;
    //     using value_type = DomainDuration;
    //     using difference_type = std::ptrdiff_t;
    //     using pointer = const DomainDuration*;
    //     using reference = const DomainDuration&;
    //
    //     Iterator(const Steps* steps, std::size_t index) : m_steps(steps), m_index(index) {}
    //
    //     reference operator*() const {
    //         if (m_index == 0) {
    //             return m_steps->m_first_delta;
    //         } else if (m_index == m_steps->m_num_steps - 1) {
    //             return m_steps->m_last_delta;
    //         }
    //
    //         return m_steps->m_delta;
    //     }
    //
    //     pointer operator->() const { return &(**this); }
    //
    //     Iterator& operator++() { ++m_index; return *this; }
    //
    //     Iterator operator++(int) {
    //         Iterator temp = *this;
    //         ++(*this);
    //         return temp;
    //     }
    //
    //     friend bool operator==(const Iterator& a, const Iterator& b) {
    //         return a.m_steps == b.m_steps && a.m_index == b.m_index;
    //     }
    //
    //     friend bool operator!=(const Iterator& a, const Iterator& b) { return !(a == b); }
    //
    // private:
    //     const Steps* m_steps = nullptr;
    //     std::size_t m_index = 0;
    // };


    Steps(const DomainDuration& duration, const DomainDuration& delta, StepRounding rounding)
        : m_rounding(rounding) {
        assert(delta.get_value() > 0);
        assert(duration.get_value() > delta.get_value());
        assert(duration.get_type() == delta.get_type());

        double r = duration.get_value();
        double q = duration.get_value() / delta.get_value();
        DomainType t = duration.get_type();

        if (rounding == StepRounding::exact_stop_before) {
            m_num_steps = static_cast<std::size_t>(std::floor(q));
            set_all(delta);
        } else if (rounding == StepRounding::exact_stop_after) {
            m_num_steps = static_cast<std::size_t>(std::ceil(q));
            set_all(delta);
        } else if (rounding == StepRounding::round_all) {
            double n = std::round(q);
            m_num_steps = static_cast<std::size_t>(n);
            set_all(DomainDuration(r / n, t));
        } else if (rounding == StepRounding::round_first || rounding == StepRounding::round_last) {
            double n = std::ceil(q);
            m_num_steps = static_cast<std::size_t>(n);
            set_all(delta);
            auto fractional_step = delta * utils::modulo(q, 1.0);


            if (rounding == StepRounding::round_first) {
                m_first_delta = fractional_step;
            } else {
                assert(m_num_steps >= 3); // lazy assertion: implementation doesn't handle this edge case
                m_last_delta = fractional_step;
            }
        }
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


    // Iterator begin() const { return Iterator(this, 0); }
    //
    // Iterator end() const { return Iterator(this, m_num_steps); }


private:
    void set_all(const DomainDuration& delta) {
        m_first_delta = delta;
        m_delta = delta;
        m_last_delta = delta;
    }


    StepRounding m_rounding;

    DomainDuration m_first_delta{0.0};
    DomainDuration m_delta{0.0};
    DomainDuration m_last_delta{0.0};

    std::size_t m_num_steps = 0;
};


// ==============================================================================================

template<typename T, typename = std::enable_if_t<std::is_base_of_v<Generative, T> > >
class GenerativeRunner {
public:
    explicit GenerativeRunner(const DomainDuration& default_step_size = DomainDuration(0.01, DomainType::ticks))
        : m_default_step_size(default_step_size) {}


    void add_generative(Generative& g) {
        if (!m_generatives.contains(&g)) {
            m_generatives.append(&g);
        }
    }


    void set_output_node(Node<T>& node) {
        auto* generative = dynamic_cast<Generative*>(&node);
        assert(generative);

        if (!m_generatives.contains(generative)) {
            m_generatives.append(generative);
        }

        m_output_node = &node;
    }


    RunResult<T> step_until(const DomainTimePoint& t
                            , StepOutput<T> previous_steps_details = StepOutput<T>::output()
                            , StepRounding step_rounding = StepRounding::round_last
                            , std::optional<DomainDuration> step_size = std::nullopt) {
        check_runner_validity();

        if (t <= m_current_time) {
            throw std::runtime_error("Cannot step backwards in time: use discontinuity() instead");
        }

        auto duration = DomainDuration::distance(m_current_time, t);
        auto steps = Steps(duration, step_size.value_or(m_default_step_size), step_rounding);

        auto outputs = Vec<StepOutput<T> >::allocated(steps.num_steps() - 1);

        for (int i = 0; i < steps.num_steps() - 1; ++i) {
            if (i == 0) {
                update_time(steps.first());
            } else {
                update_time(steps.default_delta());
            }

            auto output = process_step();

            if (!output.success) {
                return RunResult<T>(output, outputs, false, steps.domain());
            }

            outputs.append(output);
        }

        update_time(steps.last());
        auto output = process_step();

        return RunResult<T>(output, outputs, output.success, steps.domain());
    }


    RunResult<T> step_for(const DomainDuration& t
                          , StepOutput<T> previous_step_handling = StepOutput<T>::output()
                          , std::optional<DomainDuration> step_size = std::nullopt) {
        throw std::runtime_error("Not implemented");
    }


    // TODO: Pass lambda
    RunResult<T> step_while() { throw std::runtime_error("Not implemented"); };

    RunResult<T> step_n(std::size_t n) { throw std::runtime_error("Not implemented"); }

    RunResult<T> discontinuity(const DomainTimePoint& new_time) { throw std::runtime_error("Not implemented"); }


    template<typename NodeValueType>
    void schedule_parameter_change(Node<NodeValueType>& node
                                   , const NodeValueType& new_value
                                   , const DomainTimePoint& time) {
        // TODO: Add node as output node if it's not already in m_generatives
        throw std::runtime_error("Not implemented");
    }


    template<typename NodeValueType>
    void schedule_parameter_ramp(Node<NodeValueType>& node
                                 , const NodeValueType& start_value
                                 , const NodeValueType& end_value
                                 , const DomainTimePoint& end_time
                                 , const std::optional<DomainTimePoint>& start_time = std::nullopt) {
        // TODO: Add node as output node if it's not already in m_generatives
        throw std::runtime_error("Not implemented");
    }


    void set_tempo() { throw std::runtime_error("Not implemented"); }

    void schedule_tempo_change() { throw std::runtime_error("Not implemented"); }

    void schedule_tempo_ramp() { throw std::runtime_error("Not implemented"); }

    void schedule_meter_change() { throw std::runtime_error("Not implemented"); }

private:
    void check_runner_validity() {
        if (m_default_step_size.get_value() <= 0.0) {
            throw std::runtime_error("Default step size must be positive");
        }

        if (!m_output_node) {
            throw std::runtime_error("Output node not set");
        }
    }


    void update_time(const DomainDuration& delta) {
        assert(delta.get_value() > 0.0);

        m_current_time += delta;

        for (auto& g: m_generatives) {
            g->update_time(m_current_time);
        }
    }


    StepResult<T> process_step(bool is_last_step, const StepOutput<T>& output_mode, DomainType& t) {
        // TODO: we will need to handle parameter / meter / time signature ramps/scheduled changes here too in the future!!
        auto output = m_output_node->process();

        // In both Output and Discard modes, it will be up to the recipient to handle discard/output strategies
        if (is_last_step || !output_mode.is_assertion()) {
            return StepResult<T>(m_current_time, output, true, t);
        }

        return StepResult<T>(m_current_time, output, output_mode.do_assert(output), t);
    }


    Vec<Generative*> m_generatives;
    Node<T>* m_output_node = nullptr;

    DomainDuration m_default_step_size;

    TimePoint m_current_time;
};

#endif //GENERATIVE_RUNNER_H
