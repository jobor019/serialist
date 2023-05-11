

#ifndef SERIALIST_LOOPER_PHASOR_H
#define SERIALIST_LOOPER_PHASOR_H

#include <cmath>
#include <iostream>
#include <iomanip>


class Phasor {
public:

    enum class Mode {
        stepped, scheduled
    };


    explicit Phasor(double step_size = 0.1
                    , double max = 1.0
                    , double phase = 0.0
                    , Mode mode = Mode::stepped
                    , double current_time = 0.0)
            : m_step_size(step_size)
              , m_max(max)
              , m_current_value(std::fmod(phase, 1.0) * max)
              , m_mode(mode)
              , m_previous_update_time(current_time)
              , m_is_first_value(true) {}


    double process(double time) {
        double increment;
        if (m_mode == Mode::stepped) {
            if (m_is_first_value) {
                m_is_first_value = false;
                return m_current_value;
            }
            increment = m_step_size;
        } else {
            increment = m_step_size * (time - m_previous_update_time);
        }
        m_previous_update_time = time;

        double val = m_current_value + increment;

        // modulo (supports negative numerator but not negative denominator)
        m_current_value = std::fmod(std::fmod(val, m_max) + m_max, m_max);

        // Handle fmod rounding errors
        if (std::abs(m_current_value - m_max) < 1e-8) {
            m_current_value = 0;
        }

        m_is_first_value = false;
        return m_current_value;
    }


    void set_phase(double value, bool reset_to = false) {
        m_is_first_value = reset_to;
        m_current_value = std::fmod(value, m_max);
    }


    void set_step_size(double value) {
        Phasor::m_step_size = value;
    }


    void set_max(double value) {
        Phasor::m_max = value;
    }


    void set_mode(Mode new_mode) {
        m_mode = new_mode;
    }


    [[nodiscard]] double get_step_size() const { return m_step_size; }


    [[nodiscard]] double get_max() const { return m_max; }


    [[nodiscard]] Mode get_mode() const { return m_mode; }


    [[nodiscard]] double get_current_value() const { return m_current_value; }


private:
    double m_step_size;
    double m_max;
    double m_current_value;
    Mode m_mode;

    double m_previous_update_time;
    bool m_is_first_value;
};

#endif //SERIALIST_LOOPER_PHASOR_H
