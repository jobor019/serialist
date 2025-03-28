#ifndef SERIALIST_SAMPLE_AND_HOLD_H
#define SERIALIST_SAMPLE_AND_HOLD_H

#include "core/event.h"
#include "core/generative.h"
#include "core/temporal/trigger.h"
#include "core/algo/facet.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "sequence.h"
#include "variable.h"


namespace serialist {
// ==============================================================================================

class SampleAndHold {
public:
    static constexpr auto DEFAULT_CLOSED_STATE = false;
    static constexpr double STATE_OPEN = 1.0;
    static constexpr double STATE_CLOSED = 0.0;


    Voice<double> process(Voice<double>&& v, bool is_closed) {
        if (is_closed) {
            return m_current_value;
        }
        m_current_value = std::move(v);
        return m_current_value;
    }

private:
    Voice<double> m_current_value;
};


// ==============================================================================================

class SampleAndHoldNode : public NodeBase<Facet> {
public:
    struct Keys {
        static const inline std::string INPUT_VALUE = "input_value";
        static const inline std::string HOLD_STATE = "hold_state";
        static const inline std::string CLASS_NAME = "sample_and_hold";
    };


    SampleAndHoldNode(const std::string& identifier
                      , ParameterHandler& parent
                      , Node<Trigger>* trigger = nullptr
                      , Node<Facet>* input_value = nullptr
                      , Node<Facet>* hold_state = nullptr
                      , Node<Facet>* enabled = nullptr
                      , Node<Facet>* num_voices = nullptr)
        : NodeBase<Facet>(identifier, parent, enabled, num_voices, Keys::CLASS_NAME)
        , m_input_value(add_socket(Keys::INPUT_VALUE, input_value))
        , m_hold_state(add_socket(Keys::HOLD_STATE, hold_state)) {}


    Voices<Facet> process() override {
        if (!pop_time()) {
            return m_current_value;
        }

        if (!is_enabled() || !m_input_value.is_connected() || !m_hold_state.is_connected()) {
            m_current_value = Voices<Facet>::empty_like();
            return m_current_value;
        }

        auto input_value = m_input_value.process();
        auto hold_state = m_hold_state.process();

        auto num_voices = voice_count(input_value.size(), hold_state.size());

        if (num_voices != m_snh.size()) {
            m_snh.resize(num_voices);
        }

        auto in = input_value.adapted_to(num_voices).as_type<double>();
        auto is_closed = hold_state
                .adapted_to(num_voices)
                .as_type<bool>([](const Facet& f) {
                    return utils::equals(static_cast<double>(f), SampleAndHold::STATE_CLOSED);
                })
                .firsts_or(false);

        auto output = Voices<Facet>::zeros(num_voices);
        for (std::size_t i = 0; i < num_voices; ++i) {
            output[i] = m_snh[i].process(std::move(in[i]), is_closed[i]).as_type<Facet>();
        }

        m_current_value = output;
        return output;
    }

private:
    Socket<Facet>& m_input_value;
    Socket<Facet>& m_hold_state;

    MultiVoiced<SampleAndHold, std::optional<double>> m_snh;

    Voices<Facet> m_current_value = Voices<Facet>::empty_like();
};


// ==============================================================================================

template<typename FloatType = double>
struct SampleAndHoldWrapper {
    using Keys = SampleAndHoldNode::Keys;

    ParameterHandler ph;

    Sequence<Facet, double> input_value{Keys::INPUT_VALUE, ph, Voices<double>::empty_like()};
    Sequence<Facet, double> hold_state{Keys::HOLD_STATE
                                       , ph
                                       , Voices<double>::singular(SampleAndHold::STATE_OPEN)
    };

    Variable<Facet, bool> enabled{param::properties::enabled, ph, true};
    Variable<Facet, std::size_t> num_voices{param::properties::num_voices, ph, 1};
    SampleAndHoldNode sample_and_hold{Keys::CLASS_NAME
                                             , ph
                                             , nullptr
                                             , &input_value
                                             , &hold_state
                                             , &enabled
                                             , &num_voices
    };
};
}


#endif //SERIALIST_SAMPLE_AND_HOLD_H
