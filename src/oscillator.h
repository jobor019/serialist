

#ifndef SERIALISTLOOPER_OSCILLATOR_H
#define SERIALISTLOOPER_OSCILLATOR_H

#include "parameter_policy.h"
#include "generative.h"
#include "phasor.h"
#include "utils.h"
#include "socket_policy.h"

class Oscillator : public Node<double> {
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


    Oscillator(const std::string& identifier
               , ParameterHandler& parent
               , Node<Type>* type = nullptr
               , Node<float>* freq = nullptr
               , Node<float>* add = nullptr
               , Node<float>* mul = nullptr
               , Node<float>* duty = nullptr
               , Node<float>* curve = nullptr
               , Node<bool>* enabled = nullptr)
            : Node<double>(identifier, parent)
              , m_type("type", *this, type)
              , m_freq("freq", *this, freq)
              , m_add("add", *this, add)
              , m_mul("mul", *this, mul)
              , m_duty("duty", *this, duty)
              , m_curve("curve", *this, curve)
              , m_enabled("enabled", *this, enabled)
              , m_previous_values(100) {
    }


    std::vector<double> process(const TimePoint& t) override {
        auto value = step_oscillator(t);
        m_previous_values.push(value);
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


    void set_type(Node<Type>* type) { m_type = type; }


    void set_freq(Node<float>* freq) { m_freq = freq; }


    void set_add(Node<float>* add) { m_add = add; }


    void set_mul(Node<float>* mul) { m_mul = mul; }


    void set_duty(Node<float>* duty) { m_duty = duty; }


    void set_curve(Node<float>* curve) { m_curve = curve; }


    void set_enabled(Node<bool>* enabled) { m_enabled = enabled; }


    std::vector<double> get_output_history() {
        return m_previous_values.pop_all();
    }


private:

    double step_oscillator(const TimePoint& t) {
        switch (m_type.process_or(t, Type::phasor)) {
            case Type::phasor:
                return phasor(t);
            case Type::sin:
                return sin(t);
            case Type::square:
            case Type::tri:
            case Type::white_noise:
            case Type::brown_noise:
            case Type::random_walk:
                throw std::runtime_error("oscillator types not implemented"); // TODO
        }
    }


    double phasor_position(const TimePoint& t) {
        auto freq = m_freq.process_or(t, 1.0f);
        auto step_size = std::abs(freq) > 1e-8 ? 1 / freq : 0.0;

        return m_phasor.process(t.get_tick(), step_size);
    }


    double phasor(const TimePoint& t) {
        return m_mul.process_or(t, 1.0f) * phasor_position(t) + m_add.process_or(t, 0.0f);
    }


    double sin(const TimePoint& t) {
        auto y = 0.5 * std::sin(2 * M_PI * phasor_position(t)) + 0.5;
        return m_mul.process_or(t, 1.0f) * y + m_add.process_or(t, 0.0f);
    }


    Socket<Type> m_type;
    Socket<float> m_freq;
    Socket<float> m_add;
    Socket<float> m_mul;
    Socket<float> m_duty;
    Socket<float> m_curve;

    Socket<bool> m_enabled;

    Phasor m_phasor;


    utils::LockingQueue<double> m_previous_values;


};

#endif //SERIALISTLOOPER_OSCILLATOR_H
