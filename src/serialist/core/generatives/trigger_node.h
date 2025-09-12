
#ifndef SERIALIST_TRIGGER_NODE_H
#define SERIALIST_TRIGGER_NODE_H
#include "sequence.h"
#include "variable.h"
#include "policies/parameter_policy.h"
#include "policies/socket_policy.h"
#include "stereotypes/base_stereotypes.h"


namespace serialist {
class TriggerNode : public NodeBase<Facet> {
public:
    struct Keys {
        static const inline std::string VALUE = "value";

        static const inline std::string CLASS_NAME = "trigger";
    };

    TriggerNode(const std::string& identifier
                , ParameterHandler& parent
                , Node<Trigger>* trigger = nullptr
                , Node<Facet>* value = nullptr
                , Node<Facet>* enabled = nullptr
                , Node<Facet>* num_voices = nullptr)
        : NodeBase<Facet>(identifier, parent, enabled, num_voices, Keys::CLASS_NAME)
        , m_trigger(add_socket(param::properties::trigger, trigger))
        , m_value(add_socket(Keys::VALUE, value)) {}


    Voices<Facet> process() override {
        if (!pop_time()) return m_current_value;

        if (!is_enabled() || !m_trigger.is_connected() || !m_value.is_connected()) {
            m_current_value = Voices<Facet>::empty_like();
            return m_current_value;
        }

        auto trigger = m_trigger.process();

        if (trigger.is_empty_like()) {
            return m_current_value;
        }

        auto value = m_value.process();

        auto num_voices = voice_count(value.size(), trigger.size());

        auto values = value.adapted_to(num_voices);
        auto triggers = trigger.adapted_to(num_voices);

        // operating directly on m_current_value to preserve values from previous cycles
        m_current_value.adapted_to(num_voices);
        for (std::size_t i = 0; i < num_voices; ++i) {
            if (Trigger::contains_pulse_on(triggers[i])) {
                m_current_value[i] = values[i];
            }
        }

        return m_current_value;
    }


private:
    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_value;

    Voices<Facet> m_current_value = Voices<Facet>::empty_like();
};


// ==============================================================================================


template<typename FloatType = double>
struct TriggerNodeWrapper {
    using Keys = TriggerNode::Keys;

    ParameterHandler ph;

    Sequence<Trigger> trigger{param::properties::trigger, ph};
    Sequence<Facet, FloatType> value{Keys::VALUE, ph};
    Variable<Facet, bool> enabled{param::properties::enabled, ph, true};
    Variable<Facet, std::size_t> num_voices{param::properties::num_voices, ph, 0};

    TriggerNode trigger_node{Keys::CLASS_NAME
                             , ph
                             , &trigger
                             , &value
                             , &enabled
                             , &num_voices
    };
};


}

#endif //SERIALIST_TRIGGER_NODE_H
