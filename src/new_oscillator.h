

#ifndef SERIALISTLOOPER_NEW_OSCILLATOR_H
#define SERIALISTLOOPER_NEW_OSCILLATOR_H

#include "parameter_policy.h"
#include "generative.h"
#include "phasor.h"
#include "utils.h"

class NewOscillator : public Node<double>, public ParameterHandler {
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


    NewOscillator(const std::string& identifier
                  , ParameterHandler& parent
                  , Node<Type>* type = nullptr
                  , Node<float>* freq = nullptr
                  , Node<float>* add = nullptr
                  , Node<float>* mul = nullptr
                  , Node<float>* duty = nullptr
                  , Node<float>* curve = nullptr
                  , Node<bool>* enabled = nullptr)
            : ParameterHandler(identifier, parent)
              , m_type(type)
              , m_freq(freq)
              , m_add(add)
              , m_mul(mul)
              , m_duty(duty)
              , m_curve(curve)
              , m_enabled(enabled)
              , m_previous_values(100) {}


    std::vector<double> process(const TimePoint& t) override {
        switch (value_or(m_type, Type::phasor, t)) {
            case Type::phasor:
                return {phasor(t)};
            case Type::sin:
                return {sin(t)};
            case Type::square:
            case Type::tri:
            case Type::white_noise:
            case Type::brown_noise:
            case Type::random_walk:
                throw std::runtime_error("oscillator types not implemented"); // TODO
        }
    }


    std::vector<Generative*> get_connected() override { // TODO: Generalize
        return collect_connected(m_type, m_freq, m_add, m_mul, m_duty, m_curve, m_enabled);
//        std::vector<Generative*> connected;
//        if (auto type = dynamic_cast<Generative*>(m_type))
//            connected.emplace_back(type);
//        if (auto freq = dynamic_cast<Generative*>(m_freq))
//            connected.emplace_back(freq);
//        if (auto add = dynamic_cast<Generative*>(m_add))
//            connected.emplace_back(add);
//        if (auto mul = dynamic_cast<Generative*>(m_mul))
//            connected.emplace_back(mul);
//        if (auto duty = dynamic_cast<Generative*>(m_duty))
//            connected.emplace_back(duty);
//        if (auto curve = dynamic_cast<Generative*>(m_curve))
//            connected.emplace_back(curve);
//        if (auto enabled = dynamic_cast<Generative*>(m_enabled))
//            connected.emplace_back(enabled);
//
//        return connected;
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

    double phasor_position(const TimePoint& t) {
        auto freq = static_cast<float>(value_or(m_freq, 1.0f, t));
        auto step_size = std::abs(freq) > 1e-8 ? 1 / freq : 0.0;

        return m_phasor.process(t.get_tick(), step_size);
    }


    double phasor(const TimePoint& t) {
        return value_or(m_mul, 1.0f, t) * phasor_position(t) + value_or(m_add, 0.0f, t);
    }


    double sin(const TimePoint& t) {
        auto y = 0.5 * std::sin(2 * M_PI * phasor_position(t)) + 0.5;
        return value_or(m_mul, 1.0f, t) * y + value_or(m_add, 0.0f, t);
    }


    Node<Type>* m_type;
    Node<float>* m_freq;
    Node<float>* m_add;
    Node<float>* m_mul;
    Node<float>* m_duty;
    Node<float>* m_curve;

    Node<bool>* m_enabled;

    Phasor m_phasor;


    utils::LockingQueue<double> m_previous_values;


};

#endif //SERIALISTLOOPER_NEW_OSCILLATOR_H
