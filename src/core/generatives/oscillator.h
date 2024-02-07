

#ifndef SERIALISTLOOPER_OSCILLATOR_H
#define SERIALISTLOOPER_OSCILLATOR_H

#include <random>

#include "core/param/parameter_policy.h"
#include "core/generative.h"
#include "core/algo/phasor.h"
#include "core/utility/enums.h"
#include "core/param/socket_policy.h"
#include "core/generatives/variable.h"
#include "core/algo/facet.h"
#include "core/param/socket_handler.h"
#include "core/algo/time/events.h"
#include "core/algo/time/time_gate.h"
#include "core/generative_stereotypes.h"
#include "core/algo/random.h"
#include "core/algo/time/filters.h"
#include "core/collections/queue.h"
#include "core/algo/voice/multi_voiced.h"
#include "unit_pulse.h"
#include "sequence.h"

class Oscillator {
public:
    enum class Type {
        phasor = 0
        , sin = 1
        , square = 2
        , tri = 3
        , white_noise = 4
        , brown_noise = 5
        , random_walk = 6
    };


    explicit Oscillator(std::optional<int> seed = std::nullopt, double tau_ticks = 0.0)
            : m_random(seed), m_lpf(tau_ticks) {}


    double process(double time, Type type, double freq, double phase, double mul
                   , double add, double duty, double curve, bool stepped, double tau, bool increment = true) {
        double x;
        if (increment) {
            x = m_phasor.process(time, freq, phase, stepped);
        } else {
            x = m_phasor.get_current_value();
        }
        auto y = mul * waveform(x, type, duty, curve) + add;
        return m_lpf.process(y, time, tau, stepped);
    }


private:
    double waveform(double x, Type type, double duty, double curve) {
        switch (type) {
            case Type::phasor:
                return x;
            case Type::sin:
                return sin(x);
            case Type::square:
                return square(x, duty);
            case Type::tri:
                return tri(x, duty, curve);
            case Type::white_noise:
                return white_noise();
            case Type::brown_noise:
                return brown_noise();
            case Type::random_walk:
                return random_walk();
            default:
                throw std::invalid_argument("oscillator types not implemented");
        }
    }


    static double sin(double x) {
        return 0.5 * -std::cos(2 * M_PI * x) + 0.5;
    }


    static double square(double x, double duty) {
        return static_cast<double>(x >= duty);
    }


    static double tri(double x, double duty, double curve) {
        if (duty < 1e-8) {                  // duty = 0 => negative phase only (avoid div0)
            return std::pow(1 - x, curve);
        } else if (x <= duty) {             // positive phase
            return std::pow(x / duty, curve);
        } else {                            // negative phase
            return std::pow(1 - (x - duty) / (1 - duty), curve);
        }
    }


    double white_noise() {
        return m_random.next();
    }


    static double brown_noise() {
        throw std::runtime_error("not implemented"); // TODO
//        double white_noise = distribution_(generator_);
//        double new_output = last_output_ + (white_noise - last_output_) / 16.0;
//        double difference = std::abs(new_output - last_output_);
//        if (difference > max_difference_) {
//            new_output = last_output_ + max_difference_ * std::copysign(1.0, new_output - last_output_);
//        }
//        last_output_ = new_output;
//        return last_output_ + 0.5;
    }


    static double random_walk() {
        throw std::runtime_error("not implemented"); // TODO
//        double next = current_ + distribution_(rng_);
//        if (next > 1.0) {
//            current_ = 2.0 - next;
//        } else if (next < 0.0) {
//            current_ = -next;
//        } else {
//            current_ = next;
//        }
//        return current_;
    }


    Phasor m_phasor;
    Random m_random;
    Smoo m_lpf;

};

// ==============================================================================================

class OscillatorNode : public NodeBase<Facet> {
public:

    static const int HISTORY_LENGTH = 300;

    class OscillatorKeys {
    public:
        OscillatorKeys() = delete;
        static const inline std::string TRIGGER = "trigger";
        static const inline std::string TYPE = "type";
        static const inline std::string FREQ = "freq";
        static const inline std::string ADD = "add";
        static const inline std::string MUL = "mul";
        static const inline std::string DUTY = "duty";
        static const inline std::string CURVE = "curve";
        static const inline std::string STEPPED = "stepped";
        static const inline std::string PHASE = "phase";
        static const inline std::string TAU = "tau";

        static const inline std::string CLASS_NAME = "oscillator";
    };


    OscillatorNode(const std::string& identifier
                   , ParameterHandler& parent
                   , Node<Trigger>* trigger = nullptr
                   , Node<Facet>* type = nullptr
                   , Node<Facet>* freq = nullptr
                   , Node<Facet>* add = nullptr
                   , Node<Facet>* mul = nullptr
                   , Node<Facet>* duty = nullptr
                   , Node<Facet>* curve = nullptr
                   , Node<Facet>* tau = nullptr
                   , Node<Facet>* phase = nullptr
                   , Node<Facet>* stepped = nullptr
                   , Node<Facet>* enabled = nullptr
                   , Node<Facet>* num_voices = nullptr)
            : NodeBase<Facet>(identifier, parent, enabled, num_voices, OscillatorKeys::CLASS_NAME)
              , m_trigger(add_socket(OscillatorKeys::TRIGGER, trigger))
              , m_type(add_socket(OscillatorKeys::TYPE, type))
              , m_freq(add_socket(OscillatorKeys::FREQ, freq))
              , m_add(add_socket(OscillatorKeys::ADD, add))
              , m_mul(add_socket(OscillatorKeys::MUL, mul))
              , m_duty(add_socket(OscillatorKeys::DUTY, duty))
              , m_curve(add_socket(OscillatorKeys::CURVE, curve))
              , m_tau(add_socket(OscillatorKeys::TAU, tau))
              , m_phase(add_socket(OscillatorKeys::PHASE, phase))
              , m_stepped(add_socket(OscillatorKeys::STEPPED, stepped)) {}


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

        auto type = m_type.process();
        auto freq = m_freq.process();
        auto mul = m_mul.process();
        auto add = m_add.process();
        auto duty = m_duty.process();
        auto curve = m_curve.process();
        auto stepped = m_stepped.process();
        auto tau = m_tau.process();
        auto phase = m_phase.process();

        auto num_voices = voice_count(trigger.size(), type.size(), freq.size(), mul.size(), add.size()
                                      , duty.size(), curve.size(), stepped.size(), phase.size(), tau.size());

        if (num_voices != m_oscillators.size()) {
            m_oscillators.resize(num_voices);
        }


        Voices<Trigger> triggers = trigger.adapted_to(num_voices);
        Vec<Oscillator::Type> types = adapt(type, num_voices, Oscillator::Type::phasor);
        Vec<double> freqs = adapt(freq, num_voices, 1.0);
        Vec<double> muls = adapt(mul, num_voices, 1.0);
        Vec<double> adds = adapt(add, num_voices, 0.0);
        Vec<double> dutys = adapt(duty, num_voices, 0.5);
        Vec<double> curves = adapt(curve, num_voices, 1.0);
        Vec<bool> steppeds = adapt(stepped, num_voices, false);
        Vec<double> taus = adapt(tau, num_voices, 0.0);
        Vec<double> phases = adapt(phase, num_voices, 0.0);

        auto output = Voices<Facet>::zeros(num_voices);
        for (std::size_t i = 0; i < num_voices; ++i) {
            bool increment = triggers[i].contains(Trigger::pulse_on);
            auto y = m_oscillators[i].process(t->get_tick(), types[i], freqs[i], phases[i], muls[i]
                                              , adds[i], dutys[i], curves[i], steppeds[i], taus[i], increment);
            output[i].append(static_cast<Facet>(y));
        }

//        m_previous_values.push(output); // TODO: Update to use Facet
        // TODO: std::move??
        m_current_value = output;

        return m_current_value;
    }


    void set_trigger(Node<Trigger>* trigger) { m_trigger = trigger; }


    void set_type(Node<Facet>* type) { m_type = type; }


    void set_freq(Node<Facet>* freq) { m_freq = freq; }


    void set_add(Node<Facet>* add) { m_add = add; }


    void set_mul(Node<Facet>* mul) { m_mul = mul; }


    void set_duty(Node<Facet>* duty) { m_duty = duty; }


    void set_curve(Node<Facet>* curve) { m_curve = curve; }


    Socket<Trigger>& get_trigger() { return m_trigger; }


    Socket<Facet>& get_type() { return m_type; }


    Socket<Facet>& get_freq() { return m_freq; }


    Socket<Facet>& get_add() { return m_add; }


    Socket<Facet>& get_mul() { return m_mul; }


    Socket<Facet>& get_duty() { return m_duty; }


    Socket<Facet>& get_curve() { return m_curve; }


    Vec<Vec<double>> get_output_history() { return m_previous_values.pop_all(); }


private:

    template<typename OutputType>
    Vec<OutputType> adapt(Voices<Facet>& values
                          , std::size_t num_voices
                          , const OutputType& default_value) {
        return values.adapted_to(num_voices).firsts_or(default_value);
    }


    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_type;
    Socket<Facet>& m_freq;
    Socket<Facet>& m_add;
    Socket<Facet>& m_mul;
    Socket<Facet>& m_duty;
    Socket<Facet>& m_curve;
    Socket<Facet>& m_tau;
    Socket<Facet>& m_phase;
    Socket<Facet>& m_stepped;


    MultiVoiced<Oscillator, double> m_oscillators;


    LockingQueue<Vec<double>> m_previous_values{HISTORY_LENGTH};

    Voices<Facet> m_current_value = Voices<Facet>::singular(Facet(0.0));
};


// ==============================================================================================

template<typename FloatType = float>
struct OscillatorWrapper {
    using Keys = OscillatorNode::OscillatorKeys;

    ParameterHandler parameter_handler;

    Sequence<Trigger> trigger{Keys::TRIGGER, parameter_handler};
    Sequence<Facet, Oscillator::Type> type{Keys::TYPE, parameter_handler
                                           , Voices<Oscillator::Type>::singular(Oscillator::Type::phasor)};
    Sequence<Facet, FloatType> freq{Keys::FREQ, parameter_handler, Voices<FloatType>::singular(0.1f)};
    Sequence<Facet, FloatType> mul{Keys::MUL, parameter_handler, Voices<FloatType>::singular(1.0f)};
    Sequence<Facet, FloatType> add{Keys::ADD, parameter_handler, Voices<FloatType>::singular(0.0f)};
    Sequence<Facet, FloatType> duty{Keys::DUTY, parameter_handler, Voices<FloatType>::singular(0.5f)};
    Sequence<Facet, FloatType> curve{Keys::CURVE, parameter_handler, Voices<FloatType>::singular(1.0f)};
    Sequence<Facet, FloatType> tau{Keys::TAU, parameter_handler, Voices<FloatType>::singular(0.0f)};
    Sequence<Facet, FloatType> phase{Keys::PHASE, parameter_handler, Voices<FloatType>::singular(0.0f)};

    Variable<Facet, bool> stepped{Keys::STEPPED, parameter_handler, true};
    Sequence<Facet, bool> enabled{ParameterKeys::ENABLED, parameter_handler, Voices<bool>::singular(true)};
    Variable<Facet, std::size_t> num_voices{ParameterKeys::NUM_VOICES, parameter_handler, 1};

    OscillatorNode oscillator{Keys::CLASS_NAME
                              , parameter_handler, &trigger, &type, &freq, &add, &mul, &duty
                              , &curve, &tau, &phase, &stepped, &enabled, &num_voices};
};

#endif //SERIALISTLOOPER_OSCILLATOR_H
