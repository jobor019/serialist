#ifndef SERIALIST_PHASE_H
#define SERIALIST_PHASE_H

#include "utility/math.h"
#include "core/types/facet.h"
#include "policies/epsilon.h"


namespace serialist {
/** A phase value in range [0, 1). */
class Phase {
public:
    enum class Direction {
        forward, backward, unchanged
    };


    explicit Phase(double phase = 0.0, double epsilon = EPSILON)
        : m_phase(phase_mod(phase, epsilon))
        , m_epsilon(epsilon) {}

    explicit Phase(const Facet& f, double epsilon = EPSILON) : Phase(f.get(), epsilon) {}


    static Phase zero(double epsilon = EPSILON) { return Phase(0.0, epsilon); }

    /** returns the largest valid value in the range [0, 1) given an epsilon */
    static Phase one(double epsilon = EPSILON) { return Phase(max(epsilon), epsilon); }


    static double phase_mod(double x, double epsilon = EPSILON) {
        return utils::modulo(x, 1.0, epsilon);
    }


    static Phase phase_mod(const Phase& x, double epsilon = EPSILON) {
        return Phase(phase_mod(x.m_phase, epsilon));
    }


    static double max(double epsilon = EPSILON) {
        return std::nextafter(wrap_point(epsilon), 0.0);
    }

    static double wrap_point(double epsilon = EPSILON) {
        return 1.0 - epsilon;
    }

    static double clip(double x, double epsilon = EPSILON) {
        return utils::clip(x, 0.0, max(epsilon));
    }


    // Alias due to the slightly confusing choice of name `abs_delta_phase`
    static double distance(const Phase& start
                           , const Phase& end
                           , std::optional<Direction> interval_direction = std::nullopt) {
        return abs_delta_phase(start, end, interval_direction);
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
    static Direction direction(const Phase& start, const Phase& end, double epsilon = 1e-6) {
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


    /** Checks whether the transition from start to end (optionally: in `interval_direction`) contains `position` */
    static bool contains(const Phase& start
                         , const Phase& end
                         , const Phase& position
                         , std::optional<Direction> interval_direction = std::nullopt
                         , bool end_inclusive = true
                         , bool start_inclusive = true) {
        if (start == end) {
            return position == start;
        }

        if (!interval_direction) {
            interval_direction = direction(start, end);
        }

        if (interval_direction == Direction::unchanged) {
            return position == start;
        }

        if (interval_direction == Direction::forward) {
            if (start.m_phase <= end.m_phase) {
                // No wrap-around case
                return utils::in(position.m_phase, start.m_phase, end.m_phase, start_inclusive, end_inclusive);
            } else {
                // Wrap-around case
                return utils::in(position.m_phase, start.m_phase, 1.0, start_inclusive, false) ||
                       utils::in(position.m_phase, 0.0, end.m_phase, true, end_inclusive);
            }
        } else { // interval_direction == Direction::backward.
            // Note that the reversed start and end inclusive flags here are intentional
            if (start.m_phase >= end.m_phase) {
                // No wrap-around case
                return utils::in(position.m_phase, end.m_phase, start.m_phase, end_inclusive, start_inclusive);
            } else {
                // Wrap-around case
                return utils::in(position.m_phase, 0.0, start.m_phase, true, start_inclusive) ||
                       utils::in(position.m_phase, end.m_phase, 1.0, end_inclusive, false);
            }
        }
    }


    /**
     * Checks whether the transition from start to end (optionally: in `interval_direction`) crosses through
     * `position` in `position_direction`.
     */
    static bool contains_directed(const Phase& start
                                  , const Phase& end
                                  , const Phase& position
                                  , Direction position_direction
                                  , std::optional<Direction> interval_direction = std::nullopt
                                  , bool end_inclusive = true
                                  , bool start_inclusive = true) {
        if (!interval_direction) {
            interval_direction = direction(start, end);
        }

        if (*interval_direction != position_direction) {
            return false;
        }

        return contains(start, end, position, interval_direction, end_inclusive, start_inclusive);
    }

    /** Returns the direction from start to end that crosses through 0.0, or Direction::unchanged if start == end */
    static Direction crossing_direction(const Phase& start, const Phase& end) {
        if (start == end) {
            return Direction::unchanged;
        }

        if (wraps_around(start, end, Direction::forward)) {
            return Direction::forward;
        }
        return Direction::backward;
    }

    static Direction inverse_direction(const Direction& direction) {
        switch (direction) {
            case Direction::forward:
                return Direction::backward;
            case Direction::backward:
                return Direction::forward;
            default:
                return Direction::unchanged;
        }
    }


    static bool wraps_around(const Phase& start
                             , const Phase& end
                             , std::optional<Direction> interval_direction = std::nullopt) {
        if (start == end) {
            return false;
        }

        if (!interval_direction) {
            interval_direction = direction(start, end);
        }

        // In the forward direction, wrap around is defined as 0 \in (start, end]_mod1.0,
        //   Example: start=0.9999, end=0.0    wraps around
        //            start=0.0,    end=0.001  does not wrap around
        //
        // In the backward direction, wrap around is defined as 0 \in [start, end)_mod1.0,
        //   Example: start=0.0, end=0.9999       wraps around
        //   Example: start=0.9999, end=0.9998    does not wrap around
        bool end_inclusive = *interval_direction == Direction::forward;
        bool start_inclusive = *interval_direction == Direction::backward;
        return contains(start, end, zero(), interval_direction, end_inclusive, start_inclusive);
    }


    double distance_to(const Phase& end, std::optional<Direction> interval_direction = std::nullopt) const {
        return distance(*this, end, interval_direction);
    }


    double distance_to(double end, std::optional<Direction> interval_direction = std::nullopt) const {
        return distance(*this, Phase{end}, interval_direction);
    }


    Phase& invert() {
        if (m_phase == 0.0) {
            m_phase = max(m_epsilon);
            return *this;
        }

        m_phase = 1 - m_phase;
        return *this;
    }


    Phase inverted() const { return Phase(*this).invert(); }


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


    explicit operator std::string() const { return "Phase(" + std::to_string(m_phase) + ")"; }

    friend std::ostream& operator<<(std::ostream& os, const Phase& phase) {
        os << static_cast<std::string>(phase) << ")";
        return os;
    }

private:
    double m_phase;
    double m_epsilon;
};
} // namespace serialist

#endif //SERIALIST_PHASE_H
