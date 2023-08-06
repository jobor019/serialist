

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

class Oscillator : public Node<Facet> {
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


    class OscillatorKeys {
    public:
        OscillatorKeys() = delete;
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
               , Node<Facet>* type = nullptr
               , Node<Facet>* freq = nullptr
               , Node<Facet>* add = nullptr
               , Node<Facet>* mul = nullptr
               , Node<Facet>* duty = nullptr
               , Node<Facet>* curve = nullptr
               , Node<Facet>* enabled = nullptr
               , Node<Facet>* num_voices = nullptr)
            : m_parameter_handler(identifier, parent)
              , m_socket_handler("", m_parameter_handler, ParameterKeys::GENERATIVE_SOCKETS_TREE)
              , m_type(OscillatorKeys::TYPE, m_socket_handler, type)
              , m_freq(OscillatorKeys::FREQ, m_socket_handler, freq)
              , m_add(OscillatorKeys::ADD, m_socket_handler, add)
              , m_mul(OscillatorKeys::MUL, m_socket_handler, mul)
              , m_duty(OscillatorKeys::DUTY, m_socket_handler, duty)
              , m_curve(OscillatorKeys::CURVE, m_socket_handler, curve)
              , m_enabled(OscillatorKeys::ENABLED, m_socket_handler, enabled)
              , m_num_voices(ParameterKeys::NUM_VOICES, m_socket_handler, num_voices)
              , m_phasors({Phasor()})
              , m_rng(std::random_device()())
              , m_distribution(0.0, 1.0)
              , m_previous_values(100)
              {
        m_parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, OscillatorKeys::CLASS_NAME);
    }


    const Voices<Facet> process(const TimePoint& t) override {
        // TODO: Only compute when trigger received!!!!
        auto num_voices = static_cast<std::size_t>(std::max(1, m_num_voices.process(t, 1).front_or(1)));
        if (num_voices != m_phasors.size()) {
            recompute_num_phasors(num_voices);
        }

        if (!static_cast<bool>(m_enabled.process(t, 1).front_or(true))) {
            return m_current_value;
        }

        std::vector<Type> types = m_type.process(t, num_voices).values_or(Type::phasor);
        std::vector<double> freqs = m_freq.process(t, num_voices).values_or(1.0);
        std::vector<double> muls = m_mul.process(t, num_voices).values_or(1.0);
        std::vector<double> adds = m_add.process(t, num_voices).values_or(0.0);
        std::vector<double> dutys = m_duty.process(t, num_voices).values_or(0.5);
        std::vector<double> curves = m_duty.process(t, num_voices).values_or(1.0);

        std::vector<Facet> output;
        output.reserve(num_voices);

        for (std::size_t i = 0; i < num_voices; ++i) {
            output.emplace_back(step_oscillator(t, i, types.at(i), freqs.at(i), muls.at(i)
                                                , adds.at(i), dutys.at(i), curves.at(i)));
        }

        m_previous_values.push(output);
        m_current_value = Voices<Facet>(output);

        return {m_current_value};
    }


    std::vector<Generative*> get_connected() override { // TODO: Generalize
        return collect_connected(m_type.get_connected()
                                 , m_freq.get_connected()
                                 , m_add.get_connected()
                                 , m_mul.get_connected()
                                 , m_duty.get_connected()
                                 , m_curve.get_connected()
                                 , m_enabled.get_connected()
                                 , m_num_voices.get_connected());
    }


    ParameterHandler& get_parameter_handler() override {
        return m_parameter_handler;
    }


    void disconnect_if(Generative& connected_to) override {
        m_type.disconnect_if(connected_to);
        m_freq.disconnect_if(connected_to);
        m_add.disconnect_if(connected_to);
        m_mul.disconnect_if(connected_to);
        m_duty.disconnect_if(connected_to);
        m_curve.disconnect_if(connected_to);
        m_enabled.disconnect_if(connected_to);
        m_num_voices.disconnect_if(connected_to);
    }


    void set_type(Node<Facet>* type) { m_type = type; }


    void set_freq(Node<Facet>* freq) { m_freq = freq; }


    void set_add(Node<Facet>* add) { m_add = add; }


    void set_mul(Node<Facet>* mul) { m_mul = mul; }


    void set_duty(Node<Facet>* duty) { m_duty = duty; }


    void set_curve(Node<Facet>* curve) { m_curve = curve; }


    void set_enabled(Node<Facet>* enabled) { m_enabled = enabled; }


    void set_num_voices(Node<Facet>* num_voices) { m_num_voices = num_voices; }


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
    ParameterHandler m_socket_handler;


    Socket<Facet> m_type;
    Socket<Facet> m_freq;
    Socket<Facet> m_add;
    Socket<Facet> m_mul;
    Socket<Facet> m_duty;
    Socket<Facet> m_curve;

    Socket<Facet> m_enabled;
    Socket<Facet> m_num_voices;

    std::vector<Phasor> m_phasors;

    std::mt19937 m_rng;
    std::uniform_real_distribution<double> m_distribution;

    utils::LockingQueue<std::vector<Facet>> m_previous_values;

    Voices<Facet> m_current_value = Voices<Facet>(Facet(1));  // NOTE: OBJECT IS NOT THREAD-SAFE!


};

#endif //SERIALISTLOOPER_OSCILLATOR_H
