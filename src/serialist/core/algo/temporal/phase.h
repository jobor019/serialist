
#ifndef SERIALIST_PHASE_H
#define SERIALIST_PHASE_H

#include "utility/math.h"

namespace serialist {

class Phase {
    /** A phase value in range [0, 1). */
public:
    static constexpr double EPSILON = 1e-8;

    explicit Phase(double phase = 0.0, double epsilon = EPSILON)
    : m_phase(phase_mod(phase, epsilon)), m_epsilon(epsilon) {}

    static double phase_mod(double x, double epsilon = EPSILON) {
        return utils::modulo(x, 1.0, epsilon);
    }

    static Phase phase_mod(const Phase& x, double epsilon = EPSILON) {
        return Phase(phase_mod(x.m_phase, epsilon));
    }

    enum class Direction { forward, backward, unknown };
    static double abs_delta_phase(const Phase& start, const Phase& end, Direction direction = Direction::unknown) {
        if (direction == Direction::forward) {
            if (start.m_phase <= end.m_phase)
                return end.m_phase - start.m_phase;
            return 1 - start.m_phase + end.m_phase;
        }

        if (direction == Direction::backward) {
            if (start.m_phase >= end.m_phase)
                return start.m_phase - end.m_phase;
            return 1 - end.m_phase + start.m_phase;
        }

        // Unknown: use the smallest distance modulo 1 between the two points
        double delta = std::abs(start.m_phase - end.m_phase);
        if (delta > 0.5)
            return 1 - delta;
        return delta;
    }

    double get() const { return m_phase; }

    explicit operator double() const { return m_phase; }

    Phase operator+(const Phase& other) const { return Phase(phase_mod(m_phase + other.m_phase, m_epsilon)); }
    Phase operator-(const Phase& other) const { return Phase(phase_mod(m_phase - other.m_phase, m_epsilon)); }

    Phase operator+(double other) const { return Phase(phase_mod(m_phase + other, m_epsilon)); }
    Phase operator-(double other) const { return Phase(phase_mod(m_phase - other, m_epsilon)); }
    Phase operator*(double other) const { return Phase(phase_mod(m_phase * other, m_epsilon)); }

    friend Phase operator+(double lhs, const Phase& rhs) { return rhs + lhs; }
    friend Phase operator-(double lhs, const Phase& rhs) { return rhs - lhs; }
    friend Phase operator*(double lhs, const Phase& rhs) { return rhs * lhs; }

    Phase& operator+=(const Phase& other) {
        m_phase = phase_mod(m_phase + other.m_phase, m_epsilon);
        return *this;
    }


    Phase& operator-=(const Phase& other) {
        m_phase = phase_mod(m_phase - other.m_phase, m_epsilon);
        return *this;
    }


    Phase& operator*=(double other) {
        m_phase = phase_mod(m_phase * other, m_epsilon);
        return *this;
    }


private:
    double m_phase;
    double m_epsilon;
};


} // namespace serialist

#endif //SERIALIST_PHASE_H
