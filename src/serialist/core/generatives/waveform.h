#ifndef SERIALIST_WAVEFORM_H
#define SERIALIST_WAVEFORM_H

#include "core/algo/random/random.h"
#include "core/types/facet.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "core/types/trigger.h"
#include "core/collections/multi_voiced.h"
#include "sequence.h"
#include "variable.h"
#include "policies/epsilon.h"
#include "types/phase.h"


namespace serialist {
// ==============================================================================================

class Waveform {
public:
    enum class Mode { phasor, sin, square, tri };

    static constexpr double DEFAULT_DUTY = 0.5;
    static constexpr double DEFAULT_CURVE = 1.0;
    static constexpr auto DEFAULT_MODE = Mode::phasor;


    explicit Waveform(double epsilon = EPSILON)
        : m_epsilon(epsilon)
        , m_max(Phase::max(epsilon)) {}


    double process(double x, Mode mode, double duty, double curve) const {
        double phase = Phase::phase_mod(x, m_epsilon);
        double y;

        switch (mode) {
            case Mode::phasor:
                y = phase;
                break;
            case Mode::sin:
                y = sin(phase);
                break;
            case Mode::square:
                y = square(phase, duty);
                break;
            case Mode::tri:
                y = tri(phase, duty, curve);
                break;
            default:
                throw std::invalid_argument("unsupported waveform type");
        }

        return scale_to_phase_range(y);
    }

private:
    double scale_to_phase_range(double phase) const {
        return phase * m_max;
    }


    static double sin(double phase) {
        return 0.5 * -std::cos(2 * M_PI * phase) + 0.5;
    }


    static double square(double phase, double duty) {
        return phase >= duty;
    }


    static double tri(double phase, double duty, double curve) {
        duty = utils::clip(duty, 0.0, 1.0);

        if (curve <= 0.0) {
            return 0.0;
        }

        if (duty < 1e-8) {                  // duty = 0 => negative phase only (avoid div0)
            return std::pow(1 - phase, curve);
        } else if (phase <= duty) {             // positive phase
            return std::pow(phase / duty, curve);
        } else {                            // negative phase
            return std::pow(1 - (phase - duty) / (1 - duty), curve);
        }
    }


    double m_epsilon;
    double m_max;
};


// ==============================================================================================

class WaveformNode : public NodeBase<Facet> {
public:
    struct Keys {
        static const inline std::string MODE = "mode";
        static const inline std::string DUTY = "duty";
        static const inline std::string CURVE = "curve";
        static const inline std::string PHASE = "phase";

        static const inline std::string CLASS_NAME = "waveform";
    };


    WaveformNode(const std::string& identifier
                 , ParameterHandler& parent
                 , Node<Trigger>* trigger = nullptr
                 , Node<Facet>* mode = nullptr
                 , Node<Facet>* duty = nullptr
                 , Node<Facet>* curve = nullptr
                 , Node<Facet>* phase = nullptr
                 , Node<Facet>* enabled = nullptr
                 , Node<Facet>* num_voices = nullptr)
        : NodeBase<Facet>(identifier, parent, enabled, num_voices, Keys::CLASS_NAME)
        , m_trigger(add_socket(Keys::MODE, trigger))
        , m_mode(add_socket(Keys::MODE, mode))
        , m_duty(add_socket(Keys::DUTY, duty))
        , m_curve(add_socket(Keys::CURVE, curve))
        , m_phase(add_socket(Keys::PHASE, phase)) {}


    Voices<Facet> process() override {
        if (!pop_time()) {
            return m_current_value;
        }

        if (!is_enabled() || !m_trigger.is_connected()) {
            m_current_value = Voices<Facet>::empty_like();
            return m_current_value;
        }

        auto trigger = m_trigger.process();
        if (trigger.is_empty_like())
            return m_current_value;

        auto mode = m_mode.process();
        auto duty = m_duty.process();
        auto curve = m_curve.process();
        auto phase = m_phase.process();

        auto num_voices = voice_count(trigger.size(), mode.size(), duty.size(), curve.size(), phase.size());

        if (num_voices != m_waveforms.size()) {
            m_waveforms.resize(num_voices);
        }

        auto triggers = trigger.adapted_to(num_voices);
        auto modes = mode.adapted_to(num_voices).firsts_or(Waveform::DEFAULT_MODE);
        auto duties = duty.adapted_to(num_voices).firsts_or(Waveform::DEFAULT_DUTY);
        auto curves = curve.adapted_to(num_voices).firsts_or(Waveform::DEFAULT_CURVE);
        auto phases = phase.adapted_to(num_voices).firsts();

        m_current_value.adapted_to(num_voices);
        for (std::size_t i = 0; i < num_voices; ++i) {
            if (phases[i].has_value() && Trigger::contains_pulse_on(triggers[i])) {
                auto f = Facet{m_waveforms[i].process(phases[i].value(), modes[i], duties[i], curves[i])};
                m_current_value[i] = Voice<Facet>::singular(f);
            }
        }

        return m_current_value;
    }

private:
    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_mode;
    Socket<Facet>& m_duty;
    Socket<Facet>& m_curve;

    Socket<Facet>& m_phase;

    MultiVoiced<Waveform, double> m_waveforms;
    Voices<Facet> m_current_value = Voices<Facet>::empty_like();
};


// ==============================================================================================

template<typename FloatType = double>
struct WaveformWrapper {
    using Keys = WaveformNode::Keys;

    ParameterHandler ph;

    Sequence<Trigger> trigger{param::properties::trigger, ph, Trigger::pulse_on()};
    Sequence<Facet, Waveform::Mode> mode{Keys::MODE, ph, Voices<Waveform::Mode>::singular(Waveform::DEFAULT_MODE)};
    Sequence<Facet, FloatType> duty{Keys::DUTY, ph, Voices<FloatType>::singular(Waveform::DEFAULT_DUTY)};
    Sequence<Facet, FloatType> curve{Keys::CURVE, ph, Voices<FloatType>::singular(Waveform::DEFAULT_CURVE)};
    Sequence<Facet, FloatType> phase{Keys::PHASE, ph, Voices<FloatType>::empty_like()};

    Variable<Facet, bool> enabled{param::properties::enabled, ph, true};
    Variable<Facet, std::size_t> num_voices{param::properties::num_voices, ph, 0};

    WaveformNode waveform{Keys::CLASS_NAME
                          , ph
                          , &trigger
                          , &mode
                          , &duty
                          , &curve
                          , &phase
                          , &enabled
                          , &num_voices
    };
};
}


#endif //SERIALIST_WAVEFORM_H
