
#ifndef SERIALISTLOOPER_RANDOM_PULSATOR_H
#define SERIALISTLOOPER_RANDOM_PULSATOR_H

#include <functional>
#include <cmath>
#include "core/algo/weighted_random.h"
#include "core/algo/time/events.h"
#include "core/algo/time/time_gate.h"
#include "core/param/parameter_policy.h"
#include "core/param/socket_handler.h"
#include "core/param/parameter_keys.h"
#include "core/generative_stereotypes.h"
#include "core/algo/time/equal_duration_sampling.h"
#include "core/algo/time/jump_gate.h"
#include "core/algo/voice/multi_voiced.h"
#include "sequence.h"
#include "variable.h"

class RandomPulsator : public Flushable<Trigger> {
public:
    static constexpr double DURATION_BOUND_THRESHOLD = 0.015625;   // 256d-note
    static constexpr double OFFSET_RATIO_THRESHOLD = 0.1;
    static constexpr double PROBABILITY_RECOMPUTE_THRESHOLD = 0.1;


    explicit RandomPulsator(double lower_bound = 0.25
                            , double upper_bound = 4.0
                            , double relative_offset = 1.0
                            , std::optional<unsigned int> seed = std::nullopt)
            : m_duration_rnd(lower_bound, upper_bound, seed)
              , m_pulse_width(utils::clip(relative_offset, {0.0}, {1.0})) {}


    Voice<Trigger> process(double time) {
        if (m_configuration_changed) {
            recompute_existing(time);
            m_configuration_changed = false;
        }

        Voice<Trigger> output;
        if (auto pulse_off = process_current_pulse_off(time)) {
            output.append(*pulse_off);
        }
        if (auto pulse_on = process_current_pulse_on(time)) {
            output.append(*pulse_on);
        }

        if (!m_next_pulse_on_time) {
            schedule_next(time);
            m_previous_trigger_time = time;
        }

        return output;

    }


    Voice<Trigger> flush() override {
        if (m_next_pulse_off_time) {
            m_next_pulse_off_time = std::nullopt;
            return Voice<Trigger>::singular(Trigger::pulse_off);
        }
        return {};
    }


    Voice<Trigger> reset_time() {
        m_next_pulse_on_time = std::nullopt;
        return flush();
    }


    void set_lower_bound(double lower_bound) {
        m_duration_rnd.set_lower_bound(lower_bound, false);
        m_configuration_changed = true;
    }


    void set_upper_bound(double upper_bound) {
        m_duration_rnd.set_upper_bound(upper_bound, false);
        m_configuration_changed = true;
    }


    void set_pulse_width(double pw) {
        m_pulse_width = utils::clip(pw, {0.0}, {1.0});
        m_configuration_changed = true;
    }


private:

    std::optional<Trigger> process_current_pulse_off(double current_time) {
        if (m_next_pulse_off_time && current_time >= *m_next_pulse_off_time) {
            m_next_pulse_off_time = std::nullopt;
            return {Trigger::pulse_off};
        }
        return std::nullopt;
    }


    std::optional<Trigger> process_current_pulse_on(double current_time) {
        if (m_next_pulse_on_time && current_time >= *m_next_pulse_on_time) {
            m_next_pulse_on_time = std::nullopt;
            return {Trigger::pulse_on};
        }
        return std::nullopt;

    }


    void schedule_next(double current_time, bool schedule_pulse_off = true) {
        auto duration = m_duration_rnd.next();
        m_next_pulse_on_time = current_time + duration;
        if (schedule_pulse_off)
            m_next_pulse_off_time = current_time + duration * m_pulse_width;
    }


    void recompute_existing(double current_time) {
        assert(current_time >= m_previous_trigger_time);

        m_duration_rnd.recompute_coefficients();

        if (conditional_recompute_full_pulse())
            return;

        conditional_recompute_pulse_off();
    }


    bool conditional_recompute_full_pulse() {
        if (!m_next_pulse_on_time)
            return false;

        auto current_duration = *m_next_pulse_on_time - m_previous_trigger_time;
        auto p_current_duration = m_duration_rnd.pdf(current_duration);

        if (p_current_duration < PROBABILITY_RECOMPUTE_THRESHOLD) {
            std::cout << "recomputing full pulse (old duration: " << current_duration << ", p=" << p_current_duration << ")\n";
            // note: new duration is generated relative to last trigger, not current time
            schedule_next(m_previous_trigger_time, !m_next_pulse_off_time.has_value());
            return true;
        }
        return false;
    }


    bool conditional_recompute_pulse_off() {
        if (!m_next_pulse_off_time || !m_next_pulse_on_time)
            return false;

        auto current_offset_duration = *m_next_pulse_off_time - m_previous_trigger_time;
        auto current_onset_duration = *m_next_pulse_on_time - m_previous_trigger_time;
        if (current_onset_duration <= 0.0) {
            return false;
        }

        auto current_offset_ratio = current_offset_duration / current_onset_duration;
        if (std::abs(current_offset_ratio - m_pulse_width) > OFFSET_RATIO_THRESHOLD) {
            m_next_pulse_off_time = m_previous_trigger_time + current_onset_duration * m_pulse_width;
            return true;
        }
        return false;
    }


    EqualDurationSampling m_duration_rnd;

    double m_pulse_width = 1.0;

    bool m_configuration_changed = false;

    double m_previous_trigger_time = 0.0;
    std::optional<double> m_next_pulse_off_time = std::nullopt;
    std::optional<double> m_next_pulse_on_time = std::nullopt;


};


// ==============================================================================================

class RandomPulsatorNode : public NodeBase<Trigger> {
public:

    class RandomPulsatorKeys {
    public:
        static const inline std::string LOWER_BOUND = "lower_bound";
        static const inline std::string UPPER_BOUND = "upper_bound";
        static const inline std::string PULSE_WIDTH = "pulse_width";

        static const inline std::string CLASS_NAME = "random_pulsator";
    };


    RandomPulsatorNode(const std::string& id
                       , ParameterHandler& parent
                       , Node<Facet>* lower_bound
                       , Node<Facet>* upper_bound
                       , Node<Facet>* pulse_width
                       , Node<Facet>* enabled
                       , Node<Facet>* num_voices)
            : NodeBase<Trigger>(id, parent, enabled, num_voices, RandomPulsatorKeys::CLASS_NAME)
              , m_lower_bound(add_socket(RandomPulsatorKeys::LOWER_BOUND, lower_bound))
              , m_upper_bound(add_socket(RandomPulsatorKeys::UPPER_BOUND, upper_bound))
              , m_pulse_width(add_socket(RandomPulsatorKeys::PULSE_WIDTH, pulse_width)) {}


    Voices<Trigger> process() override {
        auto t = pop_time();
        if (!t) // process has already been called this cycle
            return m_current_value;

        // TODO: Enabled shouldn't be a single value but per voice!!!!!
        if (!is_enabled()) {
            if (m_previous_enable_state) {
                m_current_value = m_pulsators.flush();
            } else {
                m_current_value = Voices<Trigger>::empty_like();
            }
            m_previous_enable_state = false;
            return m_current_value;
        }
        m_previous_enable_state = true;

        auto num_voices = voice_count(m_lower_bound.voice_count()
                                      , m_upper_bound.voice_count()
                                      , m_pulse_width.voice_count());

        bool resized = false;
        Voices<Trigger> output = Voices<Trigger>::zeros(num_voices);
        if (num_voices != m_pulsators.size()) {
            output.merge_uneven(m_pulsators.resize(num_voices), true);
            resized = true;
        }


        if (m_jump_gate.poll(*t)) {
            output.merge_uneven(m_pulsators.flush(), true);
            for (auto& pulsator: m_pulsators) {
                pulsator.reset_time();
            }
        }

        if (resized || m_lower_bound.has_changed()) {
            auto lower_bounds = m_lower_bound.process().adapted_to(num_voices).firsts_or(0.25);
            m_pulsators.set(&RandomPulsator::set_lower_bound, lower_bounds.as_type<double>());
        }

        if (resized || m_upper_bound.has_changed()) {
            auto upper_bounds = m_upper_bound.process().adapted_to(num_voices).firsts_or(4.0);
            m_pulsators.set(&RandomPulsator::set_upper_bound, upper_bounds.as_type<double>());
        }

        if (resized || m_pulse_width.has_changed()) {
            auto durations = m_pulse_width.process().adapted_to(num_voices).firsts_or(1.0);
            m_pulsators.set(&RandomPulsator::set_pulse_width, durations.as_type<double>());
        }

        auto time = t->get_tick();
        for (std::size_t i = 0; i < m_pulsators.size(); ++i) {
            output[i].extend(m_pulsators[i].process(time));
        }

        m_current_value = output;
        return m_current_value;
    }


private:
    JumpGate m_jump_gate;


    Socket<Facet>& m_lower_bound;
    Socket<Facet>& m_upper_bound;
    Socket<Facet>& m_pulse_width;

    MultiVoiced<RandomPulsator, Trigger> m_pulsators;

    Voices<Trigger> m_current_value = Voices<Trigger>::empty_like();

    bool m_previous_enable_state = true;
};


// ==============================================================================================

template<typename FloatType = float>
struct RandomPulsatorWrapper {
    ParameterHandler parameter_handler;

    Sequence<Facet, FloatType> lower_bound{RandomPulsatorNode::RandomPulsatorKeys::LOWER_BOUND
                                       , parameter_handler
                                       , Voices<FloatType>::singular(0.25f)};
    Sequence<Facet, FloatType> upper_bound{RandomPulsatorNode::RandomPulsatorKeys::UPPER_BOUND
                                       , parameter_handler
                                       , Voices<FloatType>::singular(2.0f)};
    Sequence<Facet, FloatType> pulse_width{RandomPulsatorNode::RandomPulsatorKeys::PULSE_WIDTH
                                       , parameter_handler
                                       , Voices<FloatType>::singular(1.0f)};

    Sequence<Facet, bool> enabled{ParameterKeys::ENABLED, parameter_handler, Voices<bool>::singular(true)};
    Variable<Facet, std::size_t> num_voices{ParameterKeys::NUM_VOICES, parameter_handler, 1};

    RandomPulsatorNode random_pulsator{"RandomPulsator", parameter_handler
                                       , &lower_bound, &upper_bound, &pulse_width
                                       , &enabled, &num_voices};
};

#endif //SERIALISTLOOPER_RANDOM_PULSATOR_H
