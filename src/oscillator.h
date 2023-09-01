

#ifndef SERIALISTLOOPER_OSCILLATOR_H
#define SERIALISTLOOPER_OSCILLATOR_H

#include <random>

#include "parameter_policy.h"
#include "generative.h"
#include "phasor.h"
#include "utils.h"
#include "socket_policy.h"
#include "variable.h"
#include "facet.h"
#include "socket_handler.h"
#include "events.h"
#include "time_gate.h"

class Oscillator : public Node<Facet> {
public:

    static const int HISTORY_LENGTH = 300;

    enum class Type {
        phasor
        , sin
        , square
        , tri
        , white_noise
        , brown_noise
        , random_walk
    };


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

        static const inline std::string CLASS_NAME = "oscillator";
    };


    Oscillator(const std::string& identifier
               , ParameterHandler& parent
               , Node<Trigger>* trigger = nullptr
               , Node<Facet>* type = nullptr
               , Node<Facet>* freq = nullptr
               , Node<Facet>* add = nullptr
               , Node<Facet>* mul = nullptr
               , Node<Facet>* duty = nullptr
               , Node<Facet>* curve = nullptr
               , Node<Facet>* enabled = nullptr
               , Node<Facet>* num_voices = nullptr)
            : m_parameter_handler(identifier, parent)
              , m_socket_handler(m_parameter_handler)
              , m_trigger(m_socket_handler.create_socket(OscillatorKeys::TRIGGER, trigger))
              , m_type(m_socket_handler.create_socket(OscillatorKeys::TYPE, type))
              , m_freq(m_socket_handler.create_socket(OscillatorKeys::FREQ, freq))
              , m_add(m_socket_handler.create_socket(OscillatorKeys::ADD, add))
              , m_mul(m_socket_handler.create_socket(OscillatorKeys::MUL, mul))
              , m_duty(m_socket_handler.create_socket(OscillatorKeys::DUTY, duty))
              , m_curve(m_socket_handler.create_socket(OscillatorKeys::CURVE, curve))
              , m_enabled(m_socket_handler.create_socket(OscillatorKeys::ENABLED, enabled))
              , m_num_voices(m_socket_handler.create_socket(ParameterKeys::NUM_VOICES, num_voices))
              , m_phasors({Phasor()})
              , m_rng(std::random_device()())
              , m_distribution(0.0, 1.0)
              , m_previous_values(HISTORY_LENGTH) {
        m_parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, OscillatorKeys::CLASS_NAME);
    }


    void update_time(const TimePoint& t) override { m_time_gate.push_time(t); }


    Voices<Facet> process() override {
        auto t = m_time_gate.pop_time();
        if (!t) // process has already been called this cycle
            return m_current_value;

        if (!is_enabled() || !m_trigger.is_connected()) {
            m_current_value = Voices<Facet>::create_empty_like();
            return m_current_value;
        }

        auto trigger = m_trigger.process();
        if (trigger.is_empty_like())
            return m_current_value;

        auto voices = m_num_voices.process();
        auto type = m_type.process();
        auto freq = m_freq.process();
        auto mul = m_mul.process();
        auto add = m_add.process();
        auto duty = m_duty.process();
        auto curve = m_curve.process();

        auto num_voices = compute_voice_count(voices, type.size(), freq.size(), mul.size(), add.size(), duty.size()
                                              , curve.size());

        if (num_voices != m_phasors.size()) {
            recompute_num_phasors(num_voices);
        }

        Voices<Trigger> triggers = trigger.adapted_to(num_voices);
        std::vector<Type> types = type.adapted_to(num_voices).values_or(Type::phasor);
        std::vector<double> freqs = freq.adapted_to(num_voices).values_or(1.0);
        std::vector<double> muls = mul.adapted_to(num_voices).values_or(1.0);
        std::vector<double> adds = add.adapted_to(num_voices).values_or(0.0);
        std::vector<double> dutys = duty.adapted_to(num_voices).values_or(0.5);
        std::vector<double> curves = curve.adapted_to(num_voices).values_or(1.0);

        std::vector<Facet> output = m_current_value.adapted_to(num_voices).fronts_or(Facet(0.0));

        for (std::size_t i = 0; i < num_voices; ++i) {
            if (Trigger::contains(triggers.at(i), Trigger::Type::pulse)) {
                double position = step_oscillator(*t, i, types.at(i), freqs.at(i), muls.at(i)
                                                  , adds.at(i), dutys.at(i), curves.at(i));
                output.at(i) = static_cast<Facet>(position);
            }
        }

        m_previous_values.push(output);
        m_current_value = Voices<Facet>(output);

        return {m_current_value};
    }


    std::vector<Generative*> get_connected() override {
        return m_socket_handler.get_connected();
    }


    ParameterHandler& get_parameter_handler() override {
        return m_parameter_handler;
    }


    void disconnect_if(Generative& connected_to) override {
        m_socket_handler.disconnect_if(connected_to);
    }


    void set_trigger(Node<Trigger>* trigger) { m_trigger = trigger; }


    void set_type(Node<Facet>* type) { m_type = type; }


    void set_freq(Node<Facet>* freq) { m_freq = freq; }


    void set_add(Node<Facet>* add) { m_add = add; }


    void set_mul(Node<Facet>* mul) { m_mul = mul; }


    void set_duty(Node<Facet>* duty) { m_duty = duty; }


    void set_curve(Node<Facet>* curve) { m_curve = curve; }


    void set_enabled(Node<Facet>* enabled) { m_enabled = enabled; }


    void set_num_voices(Node<Facet>* num_voices) { m_num_voices = num_voices; }


    Socket<Trigger>& get_trigger() { return m_trigger; }


    Socket<Facet>& get_type() { return m_type; }


    Socket<Facet>& get_freq() { return m_freq; }


    Socket<Facet>& get_add() { return m_add; }


    Socket<Facet>& get_mul() { return m_mul; }


    Socket<Facet>& get_duty() { return m_duty; }


    Socket<Facet>& get_curve() { return m_curve; }


    Socket<Facet>& get_enabled() { return m_enabled; }


    Socket<Facet>& get_num_voices() { return m_num_voices; }


    std::vector<std::vector<Facet>> get_output_history() { return m_previous_values.pop_all(); }


private:

    bool is_enabled() { return m_enabled.process(1).front_or(true); }

    void recompute_num_phasors(std::size_t num_voices) {
        auto diff = static_cast<long>(num_voices - m_phasors.size());
        if (diff < 0) {
            // remove N last phasors
            m_phasors.erase(m_phasors.end() + diff, m_phasors.end());
        } else if (diff > 0) {
            // duplicate last phasor N times
            auto last = m_phasors.back();
            m_phasors.resize(m_phasors.size() + static_cast<std::size_t>(diff), last);
        }
    }


    double step_oscillator(const TimePoint& t, std::size_t voice_index
                           , Type type, double freq, double mul, double add, double duty, double curve) {
        auto x = phasor_position(t, voice_index, freq);
        return mul * waveform(x, type, duty, curve) + add;
    }


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


    double phasor_position(const TimePoint& t, std::size_t voice_index, double freq) {
        return m_phasors.at(voice_index).process(t.get_tick(), freq);
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


    ParameterHandler m_parameter_handler;
    SocketHandler m_socket_handler;


    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_type;
    Socket<Facet>& m_freq;
    Socket<Facet>& m_add;
    Socket<Facet>& m_mul;
    Socket<Facet>& m_duty;
    Socket<Facet>& m_curve;

    Socket<Facet>& m_enabled;
    Socket<Facet>& m_num_voices;

    std::vector<Phasor> m_phasors;

    std::mt19937 m_rng;
    std::uniform_real_distribution<double> m_distribution;

    utils::LockingQueue<std::vector<Facet>> m_previous_values;

    Voices<Facet> m_current_value = Voices<Facet>(Facet(1));  // NOTE: OBJECT IS NOT THREAD-SAFE!
    TimeGate m_time_gate;


};

#endif //SERIALISTLOOPER_OSCILLATOR_H
