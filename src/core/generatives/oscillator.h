

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
#include "core/node_base.h"

class Oscillator {
public:
    enum class Type {
        phasor
        , sin
        , square
        , tri
        , white_noise
        , brown_noise
        , random_walk
    };


    double process(double time, Type type, double freq, double mul, double add
                   , double duty, double curve, bool stepped) {
        auto x = m_phasor.process(time, freq, stepped);
        return mul * waveform(x, type, duty, curve) + add;
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
                throw std::runtime_error("oscillator types not implemented");
        }
    }


    static double sin(double x) {
        return 0.5 * -std::cos(2 * M_PI * x) + 0.5;
    }


    static double square(double x, double duty) {
        return static_cast<double>(x <= duty);
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
        return m_distribution(m_rng);
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

    std::mt19937 m_rng{std::random_device()()};
    std::uniform_real_distribution<double> m_distribution{0.0, 1.0};

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
        static const inline std::string ENABLED = "enabled";
        static const inline std::string STEPPED = "stepped";

        static const inline std::string CLASS_NAME = "oscillator";
    };


    OscillatorNode(const std::string& identifier
                   , ParameterHandler& parent
                   , Node<TriggerEvent>* trigger = nullptr
                   , Node<Facet>* type = nullptr
                   , Node<Facet>* freq = nullptr
                   , Node<Facet>* add = nullptr
                   , Node<Facet>* mul = nullptr
                   , Node<Facet>* duty = nullptr
                   , Node<Facet>* curve = nullptr
                   , Node<Facet>* enabled = nullptr
                   , Node<Facet>* stepped = nullptr
                   , Node<Facet>* num_voices = nullptr)
            : NodeBase<Facet>(identifier, parent, enabled, num_voices, OscillatorKeys::CLASS_NAME)
              , m_trigger(add_socket(OscillatorKeys::TRIGGER, trigger))
              , m_type(add_socket(OscillatorKeys::TYPE, type))
              , m_freq(add_socket(OscillatorKeys::FREQ, freq))
              , m_add(add_socket(OscillatorKeys::ADD, add))
              , m_mul(add_socket(OscillatorKeys::MUL, mul))
              , m_duty(add_socket(OscillatorKeys::DUTY, duty))
              , m_curve(add_socket(OscillatorKeys::CURVE, curve))
              , m_stepped(add_socket(OscillatorKeys::STEPPED, stepped)) {}


    Voices<Facet> process() override {
        auto t = pop_time();
        if (!t) // process has already been called this cycle
            return m_current_value;

        if (!is_enabled() || !m_trigger.is_connected()) {
            m_current_value = Voices<Facet>::create_empty_like();
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

        auto num_voices = voice_count(type.size(), freq.size(), mul.size(), add.size(), duty.size(), curve.size());

        if (num_voices != m_oscillators.size()) {
            resize(m_oscillators, num_voices);
        }

        Voices<TriggerEvent> triggers = trigger.adapted_to(num_voices);
        std::vector<Oscillator::Type> types = adapt(type, num_voices, Oscillator::Type::phasor);
        std::vector<double> freqs = adapt(freq, num_voices, 1.0);
        std::vector<double> muls = adapt(mul, num_voices, 1.0);
        std::vector<double> adds = adapt(add, num_voices, 0.0);
        std::vector<double> dutys = adapt(duty, num_voices, 0.5);
        std::vector<double> curves = adapt(curve, num_voices, 1.0);
        std::vector<bool> steppeds = adapt(stepped, num_voices, false);

        auto output = process_oscillators(*t, num_voices, triggers, types, freqs, muls, adds, dutys, curves, steppeds);

        m_previous_values.push(output);
        m_current_value = Voices<Facet>(output);

        return {m_current_value};
    }


    std::vector<Facet> process_oscillators(const TimePoint& t
                                           , std::size_t num_voices
                                           , Voices<TriggerEvent> triggers
                                           , std::vector<Oscillator::Type> types
                                           , std::vector<double> freqs
                                           , std::vector<double> muls
                                           , std::vector<double> adds
                                           , std::vector<double> dutys
                                           , std::vector<double> curves
                                           , std::vector<bool> steppeds) {
        std::vector<Facet> output = m_current_value.adapted_to(num_voices).firsts_or(Facet(0.0));

        for (std::size_t i = 0; i < num_voices; ++i) {
            if (TriggerEvent::contains(triggers.at(i), TriggerEvent::Type::pulse_on)) {
                double position = m_oscillators.at(i).process(t.get_tick(), types.at(i), freqs.at(i), muls.at(i)
                                                              , adds.at(i), dutys.at(i), curves.at(i), steppeds.at(i));
                output.at(i) = static_cast<Facet>(position);
            }
        }
        return output;
    }


    void set_trigger(Node<TriggerEvent>* trigger) { m_trigger = trigger; }
    void set_type(Node<Facet>* type) { m_type = type; }
    void set_freq(Node<Facet>* freq) { m_freq = freq; }
    void set_add(Node<Facet>* add) { m_add = add; }
    void set_mul(Node<Facet>* mul) { m_mul = mul; }
    void set_duty(Node<Facet>* duty) { m_duty = duty; }
    void set_curve(Node<Facet>* curve) { m_curve = curve; }


    Socket<TriggerEvent>& get_trigger() { return m_trigger; }
    Socket<Facet>& get_type() { return m_type; }
    Socket<Facet>& get_freq() { return m_freq; }
    Socket<Facet>& get_add() { return m_add; }
    Socket<Facet>& get_mul() { return m_mul; }
    Socket<Facet>& get_duty() { return m_duty; }
    Socket<Facet>& get_curve() { return m_curve; }


    std::vector<std::vector<Facet>> get_output_history() { return m_previous_values.pop_all(); }


private:

    template<typename OutputType>
    std::vector<OutputType> adapt(Voices<Facet> values
                                  , std::size_t num_voices
                                  , const OutputType& default_value) {
        return values.adapted_to(num_voices).firsts_or(default_value);
    }

    Socket<TriggerEvent>& m_trigger;
    Socket<Facet>& m_type;
    Socket<Facet>& m_freq;
    Socket<Facet>& m_add;
    Socket<Facet>& m_mul;
    Socket<Facet>& m_duty;
    Socket<Facet>& m_curve;
    Socket<Facet>& m_stepped;


    std::vector<Oscillator> m_oscillators{Oscillator()};


    utils::LockingQueue<std::vector<Facet>> m_previous_values{HISTORY_LENGTH};

    Voices<Facet> m_current_value = Voices<Facet>(Facet(1));  // NOTE: OBJECT IS NOT THREAD-SAFE!
};

#endif //SERIALISTLOOPER_OSCILLATOR_H
