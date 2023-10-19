

#ifndef SERIALIST_LOOPER_PHASOR_H
#define SERIALIST_LOOPER_PHASOR_H

#include <cmath>
#include <iostream>
#include <iomanip>


class Phasor {
public:

    explicit Phasor(double max = 1.0
                    , double phase = 0.0
                    , double current_time = 0.0)
            :  m_max(max)
              , m_current_value(std::fmod(phase, 1.0) * max)
              , m_previous_update_time(current_time) {}


    double process(double time, double step_size, bool stepped) {
        double increment;
        if (stepped) {
            if (m_is_first_value) {
                m_is_first_value = false;
                return m_current_value;
            }
            increment = step_size;
        } else {
            increment = step_size * (time - m_previous_update_time);
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



    [[nodiscard]] double get_max() const { return m_max; }


    [[nodiscard]] double get_current_value() const { return m_current_value; }


private:
    double m_max;
    double m_current_value;

    double m_previous_update_time;
    bool m_is_first_value = true;
};

#endif //SERIALIST_LOOPER_PHASOR_H
