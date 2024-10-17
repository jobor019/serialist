
#ifndef SERIALIST_PHASE_H
#define SERIALIST_PHASE_H

#include "utility/math.h"

namespace serialist {

/** A phase value in range [0, 1). */
class Phase {
public:
    enum class Direction {
        forward, backward, unchanged
    };


    static constexpr double EPSILON = 1e-8;


    explicit Phase(double phase = 0.0, double epsilon = EPSILON)
            : m_phase(phase_mod(phase, epsilon)), m_epsilon(epsilon) {}


    static double phase_mod(double x, double epsilon = EPSILON) {
        return utils::modulo(x, 1.0, epsilon);
    }


    static Phase phase_mod(const Phase& x, double epsilon = EPSILON) {
        return Phase(phase_mod(x.m_phase, epsilon));
    }

    static double max(double epsilon = EPSILON) {
        return std::nextafter(1.0 - epsilon, 0.0);
    }


    static double abs_delta_phase(const Phase& start
                                  , const Phase& end
                                  , std::optional<Direction> interval_direction = std::nullopt) {
        if (!interval_direction) {
            interval_direction = direction(start, end);
        }

        if (interval_direction == Direction::unchanged) {
            return 0.0;
        }

        double delta = end.m_phase - start.m_phase;

        if (interval_direction == Direction::forward) {
            return (delta >= 0) ? delta : (1.0 + delta);
        } else { // interval_direction == Direction::backward
            return (delta <= 0) ? -delta : (1.0 - delta);
        }
    }


    /** Estimate the direction based on closest distance between start and end (modulo 1.0) */
    static Direction direction(const Phase& start, const Phase& end, double epsilon=1e-6) {
        double delta = end.m_phase - start.m_phase; // delta is in (-1.0, 1.0)
        if (utils::equals(delta, 0.0, epsilon)) {
            return Direction::unchanged;
        }

        if (delta < -0.5 || utils::in(delta, 0.0, 0.5, true, true))
            // delta < -0.5 implies a wrapped around positive delta
            return Direction::forward;

        // similarly, delta >= 0.5 implies a wrapped around negative delta
        return Direction::backward;
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

    Phase& operator+=(double other) {
        m_phase = phase_mod(m_phase + other, m_epsilon);
        return *this;
    }

    Phase& operator-=(double other) {
        m_phase = phase_mod(m_phase - other, m_epsilon);
        return *this;
    }


    Phase& operator*=(double other) {
        m_phase = phase_mod(m_phase * other, m_epsilon);
        return *this;
    }

    bool operator==(const Phase& other) const { return utils::equals(m_phase, other.m_phase, m_epsilon); }
    bool operator!=(const Phase& other) const { return !operator==(other); }

    bool operator==(double other) const { return utils::equals(m_phase, other, m_epsilon); }
    bool operator!=(double other) const { return !operator==(other); }


private:
    double m_phase;
    double m_epsilon;
};


} // namespace serialist

#endif //SERIALIST_PHASE_H
