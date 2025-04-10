
#ifndef SERIALIST_LOWPASS_H
#define SERIALIST_LOWPASS_H

#include "core/temporal/filters.h"
#include "core/types/facet.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "core/types/trigger.h"
#include "core/collections/multi_voiced.h"
#include "sequence.h"
#include "variable.h"


namespace serialist {
// ==============================================================================================

class LowPassNode : public NodeBase<Facet> {
public:
    struct Keys {
        static const inline std::string INPUT = "input";
        static const inline std::string TAU = "tau";
        static const inline std::string TAU_TYPE = "tau_type";
        static const inline std::string IS_STEPPED = "is_stepped";
        static const inline std::string RESET = "reset";

        static const inline std::string CLASS_NAME = "lowpass";
    };

    LowPassNode(const std::string& identifier
                , ParameterHandler& parent
                , Node<Trigger>* trigger = nullptr
                , Node<Facet>* input = nullptr
                , Node<Facet>* tau = nullptr
                , Node<Facet>* tau_type = nullptr
                , Node<Facet>* is_stepped = nullptr
                , Node<Trigger>* reset = nullptr
                , Node<Facet>* enabled = nullptr
                , Node<Facet>* num_voices = nullptr)
        : NodeBase(identifier, parent, enabled, num_voices, Keys::CLASS_NAME)
        , m_trigger(add_socket(param::properties::trigger, trigger))
        , m_input(add_socket(Keys::INPUT, input))
        , m_tau(add_socket(Keys::TAU, tau))
        , m_tau_type(add_socket(Keys::TAU_TYPE, tau_type))
        , m_is_stepped(add_socket(Keys::IS_STEPPED, is_stepped))
        , m_reset(add_socket(Keys::RESET, reset)) {}

    Voices<Facet> process() override {
        auto t = pop_time();
        if (!t)
            return m_current_value;

        if (!is_enabled() || !m_trigger.is_connected() || !m_input.is_connected()) {
            m_current_value = Voices<Facet>::empty_like();
            return m_current_value;
        }

        auto trigger = m_trigger.process();
        if (trigger.is_empty_like())
            return m_current_value;


        auto input = m_input.process();
        auto tau = m_tau.process();

        auto tau_type = m_tau_type.process().first_or(LowPass::DEFAULT_TAU_TYPE);
        auto is_stepped = m_is_stepped.process().first_or(LowPass::DEFAULT_UNIT_STEP);

        if (Trigger::contains_pulse_on(m_reset.process())) {
            for (auto& filter : m_filters)
                filter.reset();
        }

        auto num_voices = voice_count(trigger.size(), input.size(), tau.size());

        if (num_voices != m_filters.size())
            m_filters.resize(num_voices);

        auto triggers = trigger.adapted_to(num_voices);
        auto inputs = input.adapted_to(num_voices).firsts();
        auto taus = tau.adapted_to(num_voices).firsts_or(LowPass::DEFAULT_TAU);

        m_current_value.adapted_to(num_voices);
        for (size_t i = 0; i < num_voices; ++i) {
            if (Trigger::contains_pulse_on(triggers[i])) {
                auto f = Facet{m_filters[i].process(*t, inputs[i], taus[i], tau_type, is_stepped)};
                m_current_value[i] = Voice<Facet>::singular(f);
            }
        }

        return m_current_value;
    }


private:
    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_input;
    Socket<Facet>& m_tau;
    Socket<Facet>& m_tau_type;
    Socket<Facet>& m_is_stepped;
    Socket<Trigger>& m_reset;

    MultiVoiced<LowPass, double> m_filters;

    Voices<Facet> m_current_value = Voices<Facet>::empty_like();
};


// ==============================================================================================

template<typename FloatType = double>
struct LowPassWrapper {
    using Keys = LowPassNode::Keys;

    ParameterHandler ph;

    Sequence<Trigger> trigger{param::properties::trigger, ph, Voices<Trigger>::empty_like()};
    Sequence<Facet, FloatType> input{Keys::INPUT, ph, Voices<FloatType>::empty_like()};
    Sequence<Facet, FloatType> tau{Keys::TAU, ph, Voices<FloatType>::singular(LowPass::DEFAULT_TAU)};
    Variable<Facet, DomainType> tau_type{Keys::TAU_TYPE, ph, LowPass::DEFAULT_TAU_TYPE};
    Variable<Facet, bool> is_stepped{Keys::IS_STEPPED, ph, LowPass::DEFAULT_UNIT_STEP};
    Sequence<Trigger> reset{Keys::RESET, ph, Voices<Trigger>::empty_like()};

    Sequence<Facet, bool> enabled{param::properties::enabled, ph, Voices<bool>::singular(true)};
    Variable<Facet, std::size_t> num_voices{param::properties::num_voices, ph, 0};

    LowPassNode lowpass{Keys::CLASS_NAME
                        , ph
                        , &trigger
                        , &input
                        , &tau
                        , &tau_type
                        , &is_stepped
                        , &reset
                        , &enabled
                        , &num_voices
    };
};



}
#endif //SERIALIST_LOWPASS_H
