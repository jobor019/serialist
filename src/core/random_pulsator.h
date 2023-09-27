
#ifndef SERIALISTLOOPER_RANDOM_PULSATOR_H
#define SERIALISTLOOPER_RANDOM_PULSATOR_H

#include <functional>
#include "stat.h"
#include "events.h"
#include "time_gate.h"
#include "parameter_policy.h"
#include "socket_handler.h"
#include "parameter_keys.h"
#include "node_base.h"




// ==============================================================================================


class RandomPulsator : public NodeBase<Trigger> {
public:

    class RandomPulsatorKeys {
    public:
        static const inline std::string LOWER_BOUND = "lower_bound";
        static const inline std::string UPPER_BOUND = "upper_bound";

        static const inline std::string CLASS_NAME = "random_pulsator";
    };


    RandomPulsator(const std::string& id
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

        auto lower_bounds = lower_bound.adapted_to(num_voices);
        auto upper_bounds = upper_bound.adapted_to(num_voices);
    }

private:
    class RandomPulse {
    public:

        static const inline double BOUND_THRESHOLD = 1e-4;


        bool process(double time) {
            if (m_next_trigger <= time) {
                m_next_trigger += m_random.next();
                return true;
            }
            return false;
        }


        bool update_lower_bound(double new_bound) {
            if (std::abs(m_random.get_lower_bound() - new_bound) < BOUND_THRESHOLD) {
                return false;
            }
            m_random.set_lower_bound(new_bound);
            return true;
        }


        bool update_upper_bound(double new_bound) {
            if (std::abs(m_random.get_upper_bound() - new_bound) < BOUND_THRESHOLD) {
                return false;
            }
            m_random.set_upper_bound(new_bound);
            return true;
        }


    private:

        static double pdf(double q, double lower_bound, double upper_bound) {
            return 2.0 * std::pow(0.5, q / lower_bound);
        }


        ContinuousWeightedRandom m_random{pdf};

        double m_next_trigger = 0.0;
    };


    Socket<Facet>& m_lower_bound;
    Socket<Facet>& m_upper_bound;

    std::vector<RandomPulse> m_random_pulses;

    Voices<Trigger> m_current_value = Voices<Trigger>::create_empty_like();
    bool m_previous_enabled_state = false; // enforce trigger queueing on first process call
};

#endif //SERIALISTLOOPER_RANDOM_PULSATOR_H
