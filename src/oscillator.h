

#ifndef SERIALISTLOOPER_OSCILLATOR_H
#define SERIALISTLOOPER_OSCILLATOR_H

#include <random>

#include "parameter_policy.h"
#include "generative.h"
#include "phasor.h"
#include "utils.h"
#include "socket_policy.h"
#include "variable.h"

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
              , m_rng(std::random_device()()), m_distribution(0.0, 1.0)
              , m_previous_values(100) {}


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


    Socket<Type>& get_type() { return m_type; }


    Socket<float>& get_freq() { return m_freq; }


    Socket<float>& get_add() { return m_add; }


    Socket<float>& get_mul() { return m_mul; }


    Socket<float>& get_duty() { return m_duty; }


    Socket<float>& get_curve() { return m_curve; }


    Socket<bool>& get_enabled() { return m_enabled; }


    std::vector<double> get_output_history() { return m_previous_values.pop_all(); }


private:

    double step_oscillator(const TimePoint& t) {
        switch (m_type.process_or(t, Type::phasor)) {
            case Type::phasor:
                return phasor(t);
            case Type::sin:
                return sin(t);
            case Type::square:
                return square(t);
            case Type::tri:
                return tri(t);
            case Type::white_noise:
                return white_noise(t);
            case Type::brown_noise:
                return brown_noise(t);
            case Type::random_walk:
                return random_walk(t);
            default:
                throw std::runtime_error("oscillator types not implemented");
        }
    }


    double phasor_position(const TimePoint& t) {
        auto freq = m_freq.process_or(t, 1.0f);
        return m_phasor.process(t.get_tick(), freq);
    }


    double phasor(const TimePoint& t) {
        return m_mul.process_or(t, 1.0f) * phasor_position(t) + m_add.process_or(t, 0.0f);
    }


    double sin(const TimePoint& t) {
        auto y = 0.5 * -std::cos(2 * M_PI * phasor_position(t)) + 0.5;
        return m_mul.process_or(t, 1.0f) * y + m_add.process_or(t, 0.0f);
    }


    double square(const TimePoint& t) {
        auto y = static_cast<double>(phasor_position(t) <= m_duty.process_or(t, 0.5f));
        return m_mul.process_or(t, 1.0f) * y + m_add.process_or(t, 0.0f);
    }


    double tri(const TimePoint& t) {
        auto duty = m_duty.process_or(t, 0.5f);
        auto curve = m_curve.process_or(t, 1.0f);
        auto x = phasor_position(t);

        double y;
        if (duty < 1e-8) {                  // duty = 0 => negative phase only (avoid div0)
            y = std::pow(1 - x, curve);
        } else if (x <= duty) {             // positive phase
            y = std::pow(x / duty, curve);
        } else {                            // negative phase
            y = std::pow(1 - (x - duty) / (1 - duty), curve);
        }

        return m_mul.process_or(t, 1.0f) * y + m_add.process_or(t, 0.0f);
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


    Socket<Type> m_type;
    Socket<float> m_freq;
    Socket<float> m_add;
    Socket<float> m_mul;
    Socket<float> m_duty;
    Socket<float> m_curve;

    Socket<bool> m_enabled;

    Phasor m_phasor;

    std::mt19937 m_rng;
    std::uniform_real_distribution<double> m_distribution;

    utils::LockingQueue<double> m_previous_values;


};

#endif //SERIALISTLOOPER_OSCILLATOR_H
