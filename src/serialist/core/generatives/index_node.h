
#ifndef SERIALIST_STEPPER_H
#define SERIALIST_STEPPER_H

#include "sequence.h"
#include "variable.h"
#include "policies/parameter_policy.h"
#include "policies/socket_policy.h"
#include "stereotypes/base_stereotypes.h"
#include "types/index.h"

namespace serialist {


class IndexHandler {
public:
    static constexpr double INITIAL_VALUE = 0;
    static constexpr std::size_t UNBOUNDED = 0;

    static constexpr std::size_t UPPER_LIMIT = 65535;

    static constexpr double DEFAULT_STRIDE = 1.0;
    static constexpr std::size_t DEFAULT_NUM_STEPS = UNBOUNDED;
    static constexpr bool DEFAULT_RESET = false;


    static std::size_t parse(std::size_t num_steps) {
        return num_steps == UNBOUNDED ? UPPER_LIMIT : num_steps;
    }

    std::size_t process(std::size_t num_steps, double stride, bool reset_on_change) {
        if (num_steps != m_previous_num_steps) {
            m_previous_num_steps = num_steps;

            if (reset_on_change || (m_current_value  && *m_current_value >= static_cast<double>(num_steps))) {
                m_current_value = initial_value(num_steps, stride);
                return static_cast<std::size_t>(*m_current_value);
            }
        }

        if (!m_current_value) {
            m_current_value = initial_value(num_steps, stride);
            return static_cast<std::size_t>(*m_current_value);
        }

        num_steps = parse(num_steps);

        *m_current_value = utils::modulo(*m_current_value + stride, static_cast<double>(num_steps));

        return utils::conditional_floor<std::size_t>(*m_current_value, EPSILON);
    }


    void reset() {
        m_current_value = std::nullopt;
    }

private:
    static double initial_value(std::size_t num_steps, double stride) {
        if (stride >= 0)  // stepping forward => wraps around at 0
            return INITIAL_VALUE;

        // stepping backwards => wraps around at num_steps - 1 or UPPER_LIMIT

        if (num_steps == UNBOUNDED) {
            return UPPER_LIMIT;
        }

        return static_cast<double>(num_steps - 1);
    }


    std::optional<double> m_current_value = std::nullopt;
    std::size_t m_previous_num_steps = UNBOUNDED;
};



// ==============================================================================================



class IndexNode : public NodeBase<Facet> {
public:
    struct Keys {
        static const inline std::string NUM_STEPS = "num_steps";
        static const inline std::string STRIDE = "stride";
        static const inline std::string RESET_ON_CHANGE = "reset_on_change";
        static const inline std::string RESET = "reset";

        static const inline std::string CLASS_NAME = "index";
    };

    IndexNode(const std::string& id
                , ParameterHandler& parent
                , Node<Trigger>* trigger = nullptr
                , Node<Facet>* num_steps = nullptr
                , Node<Facet>* stride = nullptr
                , Node<Facet>* reset_on_change = nullptr
                , Node<Trigger>* reset = nullptr
                , Node<Facet>* enabled = nullptr
                , Node<Facet>* num_voices = nullptr)
    : NodeBase(id, parent, enabled, num_voices, Keys::CLASS_NAME)
            , m_trigger(add_socket(param::properties::trigger, trigger))
            , m_num_steps(add_socket(Keys::NUM_STEPS, num_steps))
            , m_stride(add_socket(Keys::STRIDE, stride))
            , m_reset_on_change(add_socket(Keys::RESET_ON_CHANGE, reset_on_change))
            , m_reset(add_socket(Keys::RESET, reset)) {}

    Voices<Facet> process() override {
        auto t = pop_time();
        if (!t)
            return m_current_value;

        if (!is_enabled() || !m_trigger.is_connected()) {
            m_current_value = Voices<Facet>::empty_like();
            return m_current_value;
        }


        auto trigger = m_trigger.process();
        if (trigger.is_empty_like())
            return m_current_value;

        auto num_voices = get_voice_count();

        if (num_voices != m_index_handlers.size()) {
            m_index_handlers.resize(num_voices);
        }

        auto num_steps = m_num_steps.process().adapted_to(num_voices).firsts_or(IndexHandler::UNBOUNDED);
        auto stride = m_stride.process().adapted_to(num_voices).firsts_or(IndexHandler::DEFAULT_STRIDE);
        auto reset_triggers = m_reset.process().adapted_to(num_voices);
        auto reset_on_change = m_reset_on_change.process().adapted_to(num_voices).firsts_or(IndexHandler::DEFAULT_RESET);
        trigger.adapted_to(num_voices);

        m_current_value.adapted_to(num_voices);

        for (std::size_t i = 0; i < num_voices; ++i) {
            if (Trigger::contains_pulse_on(reset_triggers[i])) {
                m_index_handlers[i].reset();
            }

            if (Trigger::contains_pulse_on(trigger[i])) {
                auto v = m_index_handlers[i].process(num_steps[i], stride[i], reset_on_change[i]);
                m_current_value[i] = {static_cast<Facet>(v)};
            }
        }

        return m_current_value;
    }


private:

    std::size_t get_voice_count() {
        return voice_count(m_trigger.voice_count()
                           , m_num_steps.voice_count()
                           , m_stride.voice_count());
    }

    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_num_steps;
    Socket<Facet>& m_stride;
    Socket<Facet>& m_reset_on_change;
    Socket<Trigger>& m_reset;

    MultiVoiced<IndexHandler, std::size_t> m_index_handlers;

    Voices<Facet> m_current_value = Voices<Facet>::empty_like();
};


// ==============================================================================================



template<typename FloatType = double>
struct IndexWrapper {
    using Keys = IndexNode::Keys;

    ParameterHandler ph;

    Sequence<Trigger> trigger{param::properties::trigger, ph};
    Sequence<Facet, std::size_t> num_steps{Keys::NUM_STEPS, ph, IndexHandler::DEFAULT_NUM_STEPS};
    Sequence<Facet, FloatType> stride{Keys::STRIDE, ph, IndexHandler::DEFAULT_STRIDE};
    Variable<Facet, bool> reset_on_change{Keys::RESET_ON_CHANGE, ph, IndexHandler::DEFAULT_RESET};
    Sequence<Trigger> reset{Keys::RESET, ph};
    Variable<Facet, bool> enabled{param::properties::enabled, ph, true};
    Variable<Facet, std::size_t> num_voices{param::properties::num_voices, ph, 0};

    IndexNode index_node{Keys::CLASS_NAME
                         , ph
                         , &trigger
                         , &num_steps
                         , &stride
                         , &reset_on_change
                         , &reset
                         , &enabled
                         , &num_voices
    };
};

}


#endif //SERIALIST_STEPPER_H
