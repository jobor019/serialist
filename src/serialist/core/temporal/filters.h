
#ifndef SERIALISTLOOPER_FILTERS_H
#define SERIALISTLOOPER_FILTERS_H

#include <cmath>
#include <optional>
#include "time_point.h"

namespace serialist {

/**
 * @brief one-pole smoothing filter for unevenly sampled data
 *
 */
class FilterSmoo {
public:
    static constexpr double DEFAULT_TAU = 0.0;
    static const inline DomainType DEFAULT_TAU_TYPE = DomainType::ticks;
    static const inline bool DEFAULT_UNIT_STEP = false;

    static constexpr double TAU_THRESHOLD = 1e-6;


    double process(const TimePoint& t0, double x) {
        if (!t1.has_value() || t0 < *t1 || m_tau.get_value() < TAU_THRESHOLD) {
            t1 = t0;
            y = x;
            return y;
        }

        auto dt = m_is_unit_stepped ? 1.0 : t0.get(m_tau.get_type()) - t1->get(m_tau.get_type());
        t1 = t0;

        auto s = std::exp(-dt / m_tau.get_value());
        y = (1 - s) * x + s * y;
        return y;
    }


    void reset() {
        y = 0;
        t1 = std::nullopt;
    }


    void set_tau(const DomainDuration& tau) {
        if (tau.get_value() < 0) {
            throw std::invalid_argument("tau must be non-negative");
        }
        m_tau = tau;
    }

    void set_unit_stepped(bool is_unit_stepped) {
        m_is_unit_stepped = is_unit_stepped;
    }


private:
    DomainDuration m_tau{DEFAULT_TAU, DEFAULT_TAU_TYPE};
    bool m_is_unit_stepped = DEFAULT_UNIT_STEP;

    double y = 0.0;
    std::optional<TimePoint> t1 = std::nullopt;
};

} // namespace serialist

#endif //SERIALISTLOOPER_FILTERS_H
