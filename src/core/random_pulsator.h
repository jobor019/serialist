
#ifndef SERIALISTLOOPER_RANDOM_PULSATOR_H
#define SERIALISTLOOPER_RANDOM_PULSATOR_H

#include <functional>
#include "core/algo/weighted_random.h"
#include "events.h"
#include "time_gate.h"
#include "parameter_policy.h"
#include "socket_handler.h"
#include "parameter_keys.h"
#include "node_base.h"

class RandomPulsator {
public:

    static constexpr double BOUND_THRESHOLD = 0.015625;   // 256d-note

    static constexpr double JUMP_THRESHOLD_TICKS = 1.0;   // quarter note


    static double pdf(double q, double lower_bound, double) {
        assert(lower_bound >= 0.0);
        return 2.0 * std::pow(0.5, q / lower_bound);
    }


    std::optional<double> process(double time, double lower_bound, double upper_bound) {
        if (update_bounds(lower_bound, upper_bound)) {
            m_next_trigger_time = recompute(time, m_previous_trigger_time, m_random);
        }

        if (jump_occurred(time, m_previous_callback_time)) {
            m_next_trigger_time = recompute(time, time, m_random);
        }

        m_previous_callback_time = time;

        if (m_next_trigger_time <= time) {
            auto trigger_time = m_next_trigger_time;

            m_next_trigger_time += m_random.next();
            m_previous_trigger_time = trigger_time;
            return trigger_time;
        }
        return std::nullopt;
    }


private:

    static bool jump_occurred(double current_time, double previous_callback_time) {
        return std::abs(current_time - previous_callback_time) > JUMP_THRESHOLD_TICKS;
    }


    static bool has_changed(double old_bound, double new_bound) {
        return std::abs(old_bound - new_bound) > BOUND_THRESHOLD;
    }


    bool update_bounds(double lower_bound, double upper_bound) {
        if (has_changed(m_random.get_lower_bound(), lower_bound)
            || has_changed(m_random.get_upper_bound(), upper_bound)) {
            std::cout << "updating bounds" << std::endl;
            m_random.set_bounds(lower_bound, upper_bound);
            return true;
        }
        return false;
    }


    static double recompute(double current_time, double previous_trigger_time, ContinuousWeightedRandom rng) {
        auto duration = rng.next();
        if (previous_trigger_time + duration < current_time) {
            return current_time;
        }
        return previous_trigger_time + duration;
    }


    ContinuousWeightedRandom m_random{pdf, 0.25, 1.0};

    double m_previous_callback_time = 0.0;

    double m_previous_trigger_time = 0.0;
    double m_next_trigger_time = 0.0;
};


// ==============================================================================================


class RandomPulsatorNode : public NodeBase<Trigger> {
public:

    class RandomPulsatorKeys {
    public:
        static const inline std::string LOWER_BOUND = "lower_bound";
        static const inline std::string UPPER_BOUND = "upper_bound";

        static const inline std::string CLASS_NAME = "random_pulsator";
    };


    RandomPulsatorNode(const std::string& id
                       , ParameterHandler& parent
                       , Node<Facet>* lower_bound
                       , Node<Facet>* upper_bound
                       , Node<Facet>* enabled
                       , Node<Facet>* num_voices)
            : NodeBase<Trigger>(id, parent, enabled, num_voices, RandomPulsatorKeys::CLASS_NAME)
              , m_lower_bound(add_socket(RandomPulsatorKeys::LOWER_BOUND, lower_bound))
              , m_upper_bound(add_socket(RandomPulsatorKeys::UPPER_BOUND, upper_bound)) {}


    Voices<Trigger> process() override {
        auto t = pop_time();
        if (!t) // process has already been called this cycle
            return m_current_value;


        if (is_enabled()) {
            m_current_value = Voices<Trigger>::create_empty_like();
        }

        auto lower_bound = m_lower_bound.process();
        auto upper_bound = m_upper_bound.process();

        auto num_voices = voice_count(lower_bound.size(), upper_bound.size());

        if (m_pulsators.size() != num_voices) {
            resize(m_pulsators, num_voices);
        }

        auto lower_bounds = adapt(lower_bound, num_voices, 0.25);
        auto upper_bounds = adapt(upper_bound, num_voices, 1.0);

        m_current_value = Voices<Trigger>(num_voices);

        for (std::size_t i = 0; i < num_voices; ++i) {
            if (auto trigger_time = m_pulsators.at(i).process(t->get_tick(), lower_bounds.at(i), upper_bounds.at(i))) {
                m_current_value.append_to(i, {*trigger_time, Trigger::Type::pulse_on, static_cast<int>(i)});
            }
        }

        return m_current_value;
    }


private:


    Socket<Facet>& m_lower_bound;
    Socket<Facet>& m_upper_bound;

    std::vector<RandomPulsator> m_pulsators{RandomPulsator()};

    Voices<Trigger> m_current_value = Voices<Trigger>::create_empty_like();
};

#endif //SERIALISTLOOPER_RANDOM_PULSATOR_H
