

#ifndef SERIALIST_LOOPER_PHASOR_H
#define SERIALIST_LOOPER_PHASOR_H

#include <cmath>
#include <iostream>
#include <iomanip>
#include "core/utility/math.h"


class PhaseAccumulator {
public:

    explicit PhaseAccumulator(double max = 1.0
                              , double phase = 0.0
                              , double current_time = 0.0)
            :  m_max(max)
              , m_current_phase(utils::modulo(phase, 1.0) * max)
              , m_previous_update_time(current_time) {}


    double process(double time, double step_size, double phase, bool stepped) {
        double increment;
        if (stepped) {
            if (m_is_first_value) {
                m_is_first_value = false;
                return with_phase(m_current_phase, phase);
            }
            increment = step_size;
        } else {
            increment = step_size * (time - m_previous_update_time);
        }
        m_previous_update_time = time;

        double val = m_current_phase + increment;

        m_current_phase = utils::modulo(val, m_max);
        m_is_first_value = false;

        return with_phase(m_current_phase, phase);
    }


    void reset(double phase = 0.0) {
        m_is_first_value = true;
        m_current_phase = phase;
    }



    [[nodiscard]] double get_max() const { return m_max; }


    [[nodiscard]] double get_current_value() const { return m_current_phase; }


private:
    double with_phase(double x, double phase) const {
        // TODO: Replace this with utils::modulo(..., m_max, epsilon), which handles rounding errors implicitly;
        auto output = utils::modulo(x + phase * m_max, m_max);

        // Handle rounding errors
        if (std::abs(output - m_max) < 1e-8) {
            return 0.0;
        }
        return output;
    }

    double m_max;
    double m_current_phase;

    double m_previous_update_time;
    bool m_is_first_value = true;
};

#endif //SERIALIST_LOOPER_PHASOR_H
