

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


    static Facet type_to_facet(const Oscillator::Type& t) {
        return Facet::from_enum(t, Type::phasor, Type::random_walk);
    }


    static Oscillator::Type facet_to_type(const Facet& facet) {
        return facet.as_enum(Type::phasor, Type::random_walk);
    }


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
               , Node<Facet>* enabled = nullptr)
            : m_parameter_handler(identifier, parent)
              , m_socket_handler("", m_parameter_handler, ParameterKeys::GENERATIVE_SOCKETS_TREE)
              , m_type(OscillatorKeys::TYPE, m_socket_handler, type)
              , m_freq(OscillatorKeys::FREQ, m_socket_handler, freq)
              , m_add(OscillatorKeys::ADD, m_socket_handler, add)
              , m_mul(OscillatorKeys::MUL, m_socket_handler, mul)
              , m_duty(OscillatorKeys::DUTY, m_socket_handler, duty)
              , m_curve(OscillatorKeys::CURVE, m_socket_handler, curve)
              , m_enabled(OscillatorKeys::ENABLED, m_socket_handler, enabled)
              , m_rng(std::random_device()()), m_distribution(0.0, 1.0)
              , m_previous_values(100) {
        m_parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, OscillatorKeys::CLASS_NAME);
    }


    std::vector<Facet> process(const TimePoint& t) override {
        auto value = step_oscillator(t);
        m_previous_values.push(static_cast<double>(value));
        return {value};
    }


    std::vector<Generative*> get_connected() override { // TODO: Generalize
        return collect_connected(m_type.get_connected()
                                 , m_freq.get_connected()
                                 , m_add.get_connected()
                                 , m_mul.get_connected()
                                 , m_duty.get_connected()
                                 , m_curve.get_connected()
                                 , m_enabled.get_connected());
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
    }


    void set_type(Node<Facet>* type) { m_type = type; }


    void set_freq(Node<Facet>* freq) { m_freq = freq; }


    void set_add(Node<Facet>* add) { m_add = add; }


    void set_mul(Node<Facet>* mul) { m_mul = mul; }


    void set_duty(Node<Facet>* duty) { m_duty = duty; }


    void set_curve(Node<Facet>* curve) { m_curve = curve; }


    void set_enabled(Node<Facet>* enabled) { m_enabled = enabled; }


    Socket<Facet>& get_type() { return m_type; }


    Socket<Facet>& get_freq() { return m_freq; }


    Socket<Facet>& get_add() { return m_add; }


    Socket<Facet>& get_mul() { return m_mul; }


    Socket<Facet>& get_duty() { return m_duty; }


    Socket<Facet>& get_curve() { return m_curve; }


    Socket<Facet>& get_enabled() { return m_enabled; }


    std::vector<double> get_output_history() { return m_previous_values.pop_all(); }


private:

    Facet step_oscillator(const TimePoint& t) {

        auto type = facet_to_type(m_type.process_or(t, type_to_facet(Type::phasor)));
        switch (type) {
            case Type::phasor:
                return Facet(phasor(t));
            case Type::sin:
                return Facet(sin(t));
            case Type::square:
                return Facet(square(t));
            case Type::tri:
                return Facet(tri(t));
            case Type::white_noise:
                return Facet(white_noise(t));
            case Type::brown_noise:
                return Facet(brown_noise(t));
            case Type::random_walk:
                return Facet(random_walk(t));
            default:
                throw std::runtime_error("oscillator types not implemented");
        }
    }


    double phasor_position(const TimePoint& t) {
        auto freq = m_freq.process_or(t, Facet(1.0));
        return m_phasor.process(t.get_tick(), static_cast<double>(freq));
    }


    double phasor(const TimePoint& t) {
        return m_mul.process_or(t, Facet(1.0)).get() * phasor_position(t) + m_add.process_or(t, Facet(0.0)).get();
    }


    double sin(const TimePoint& t) {
        auto y = 0.5 * -std::cos(2 * M_PI * phasor_position(t)) + 0.5;
        return m_mul.process_or(t, Facet(1.0)).get() * y + m_add.process_or(t, Facet(0.0)).get();
    }


    double square(const TimePoint& t) {
        auto y = static_cast<double>(phasor_position(t) <= m_duty.process_or(t, Facet(0.5)).get());
        return m_mul.process_or(t, Facet(1.0)).get() * y + m_add.process_or(t, Facet(0.0)).get();
    }


    double tri(const TimePoint& t) {
        auto duty = m_duty.process_or(t, Facet(0.5)).get();
        auto curve = m_curve.process_or(t, Facet(1.0)).get();
        auto x = phasor_position(t);

        double y;
        if (duty < 1e-8) {                  // duty = 0 => negative phase only (avoid div0)
            y = std::pow(1 - x, curve);
        } else if (x <= duty) {             // positive phase
            y = std::pow(x / duty, curve);
        } else {                            // negative phase
            y = std::pow(1 - (x - duty) / (1 - duty), curve);
        }

        return m_mul.process_or(t, Facet(1.0)).get() * y + m_add.process_or(t, Facet(0.0)).get();
    }


    double white_noise(const TimePoint& t) {
        (void) t;
        return m_distribution(m_rng);
    }


    double brown_noise(const TimePoint& t) {
        (void) t;
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


    double random_walk(const TimePoint& t) {
        (void) t;
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

    Phasor m_phasor;

    std::mt19937 m_rng;
    std::uniform_real_distribution<double> m_distribution;

    utils::LockingQueue<double> m_previous_values;


};

#endif //SERIALISTLOOPER_OSCILLATOR_H
