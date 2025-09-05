
#ifndef SERIALIST_SCALER_H
#define SERIALIST_SCALER_H

#include "core/types/facet.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "core/types/trigger.h"
#include "sequence.h"
#include "variable.h"
#include "utility/math.h"

namespace serialist {
class Scaler {
public:
    static constexpr double DEFAULT_INPUT_LOW = 0.0;
    static constexpr double DEFAULT_INPUT_HIGH = 1.0;
    static constexpr double DEFAULT_OUTPUT_LOW = 0.0;
    static constexpr double DEFAULT_OUTPUT_HIGH = 1.0;

    static Voice<Facet> scale(const Voice<Facet>& values, double input_low, double input_high, double output_low, double output_high) {
        if (values.empty()) {
            return {};
        }

        if (utils::equals(input_low, input_high))
            return Voice<Facet>::repeated(values.size(), Facet{input_low * (output_high - output_low) + output_low});

        return values.as_type<double>()
                .clip(input_low, input_high)
                .divide(input_high - input_low)
                .multiply(output_high - output_low)
                .add(output_low)
                .as_type<Facet>();
    }
};

// ==============================================================================================

class ScalerNode : public NodeBase<Facet> {
public:
    struct Keys {
        static const inline std::string VALUE = "value";
        static const inline std::string INPUT_LOW = "input_low";
        static const inline std::string INPUT_HIGH = "input_high";
        static const inline std::string OUTPUT_LOW = "output_low";
        static const inline std::string OUTPUT_HIGH = "output_high";

        static const inline std::string CLASS_NAME = "scaler";
    };

    ScalerNode(const std::string& identifier
               , ParameterHandler& parent
               , Node<Trigger>* trigger = nullptr
               , Node<Facet>* value = nullptr
               , Node<Facet>* input_low = nullptr
               , Node<Facet>* input_high = nullptr
               , Node<Facet>* output_low = nullptr
               , Node<Facet>* output_high = nullptr
               , Node<Facet>* enabled = nullptr
               , Node<Facet>* num_voices = nullptr)
    : NodeBase<Facet>(identifier, parent, enabled, num_voices, Keys::CLASS_NAME)
    , m_trigger(add_socket(param::properties::trigger, trigger))
    , m_value(add_socket(Keys::VALUE, value))
    , m_input_low(add_socket(Keys::INPUT_LOW, input_low))
    , m_input_high(add_socket(Keys::INPUT_HIGH, input_high))
    , m_output_low(add_socket(Keys::OUTPUT_LOW, output_low))
    , m_output_high(add_socket(Keys::OUTPUT_HIGH, output_high)) {}


    Voices<Facet> process() override {
        if (!pop_time()) return m_current_value;

        if (!is_enabled() || !m_trigger.is_connected() || !m_value.is_connected()) {
            m_current_value = Voices<Facet>::singular(Facet(0.0));
            return m_current_value;
        }

        if (m_trigger.process().is_empty_like())
            return m_current_value;

        auto value = m_value.process();
        auto input_low = m_input_low.process();
        auto input_high = m_input_high.process();
        auto output_low = m_output_low.process();
        auto output_high = m_output_high.process();

        auto num_voices = voice_count(value.size(), input_low.size(), input_high.size(), output_low.size(), output_high.size());

        auto values = value.adapted_to(num_voices);
        auto input_lows = input_low.adapted_to(num_voices).firsts_or(Scaler::DEFAULT_INPUT_LOW);
        auto input_highs = input_high.adapted_to(num_voices).firsts_or(Scaler::DEFAULT_INPUT_HIGH);
        auto output_lows = output_low.adapted_to(num_voices).firsts_or(Scaler::DEFAULT_OUTPUT_LOW);
        auto output_highs = output_high.adapted_to(num_voices).firsts_or(Scaler::DEFAULT_OUTPUT_HIGH);

        auto output = Voices<Facet>::zeros(num_voices);
        for (std::size_t i = 0; i < num_voices; ++i) {
            output[i] = Scaler::scale(values[i], input_lows[i], input_highs[i], output_lows[i], output_highs[i]);
        }

        m_current_value = std::move(output);
        return m_current_value;
    }


private:
    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_value;
    Socket<Facet>& m_input_low;
    Socket<Facet>& m_input_high;
    Socket<Facet>& m_output_low;
    Socket<Facet>& m_output_high;

    Voices<Facet> m_current_value = Voices<Facet>::singular(Facet(0.0));
};


// ==============================================================================================

template<typename FloatType = double>
struct ScalerWrapper {
    using Keys = ScalerNode::Keys;

    ParameterHandler ph;
    Sequence<Trigger> trigger{param::properties::trigger, ph, Trigger::pulse_on()};
    Sequence<Facet, FloatType> value{Keys::VALUE, ph};
    Sequence<Facet, FloatType> input_low{Keys::INPUT_LOW, ph, Voices<FloatType>::singular(Scaler::DEFAULT_INPUT_LOW)};
    Sequence<Facet, FloatType> input_high{Keys::INPUT_HIGH, ph, Voices<FloatType>::singular(Scaler::DEFAULT_INPUT_HIGH)};
    Sequence<Facet, FloatType> output_low{Keys::OUTPUT_LOW, ph, Voices<FloatType>::singular(Scaler::DEFAULT_OUTPUT_LOW)};
    Sequence<Facet, FloatType> output_high{Keys::OUTPUT_HIGH, ph, Voices<FloatType>::singular(Scaler::DEFAULT_OUTPUT_HIGH)};
    Variable<Facet, bool> enabled{param::properties::enabled, ph, true};
    Variable<Facet, std::size_t> num_voices{param::properties::num_voices, ph, 0};

    ScalerNode scaler_node{Keys::CLASS_NAME
                           , ph
                           , &trigger
                           , &value
                           , &input_low
                           , &input_high
                           , &output_low
                           , &output_high
                           , &enabled
                           , &num_voices
    };
};

}



#endif //SERIALIST_SCALER_H
