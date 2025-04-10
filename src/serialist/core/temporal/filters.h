#ifndef SERIALISTLOOPER_FILTERS_H
#define SERIALISTLOOPER_FILTERS_H

#include <cmath>
#include <optional>
#include "core/types/time_point.h"


namespace serialist {
/**
 * @brief one-pole smoothing filter for (temporally) unevenly sampled data
 *
 */
class LowPass {
public:
    static constexpr double DEFAULT_TAU = 0.0;
    static constexpr auto DEFAULT_TAU_TYPE = DomainType::ticks;
    static constexpr bool DEFAULT_UNIT_STEP = false;

    static constexpr double TAU_THRESHOLD = 1e-6;


    double process(const TimePoint& t0
                   , std::optional<double> x
                   , const double& tau
                   , const DomainType& tau_type
                   , bool is_unit_stepped) {
        set_unit_stepped(is_unit_stepped);
        set_tau(DomainDuration{tau, tau_type});
        return process(t0, x.value_or(m_x_prev));
    }


    double process(const TimePoint& t0, double x) {
        if (!m_t1.has_value() || t0 < *m_t1 || m_tau.get_value() < TAU_THRESHOLD) {
            m_t1 = t0;
            m_y = x;
            m_x_prev = x;
            return m_y;
        }

        auto dt = m_is_unit_stepped ? 1.0 : t0.get(m_tau.get_type()) - m_t1->get(m_tau.get_type());
        m_t1 = t0;

        auto s = std::exp(-dt / m_tau.get_value());
        m_y = (1 - s) * x + s * m_y;

        m_x_prev = x;

        return m_y;
    }


    void reset() {
        m_y = 0;
        m_x_prev = 0;
        m_t1 = std::nullopt;
    }


    void set_tau(const DomainDuration& tau) {
        if (tau.get_value() < 0) {
            m_tau = DomainDuration(0, tau.get_type());
        } else {
            m_tau = tau;
        }
    }


    void set_unit_stepped(bool is_unit_stepped) {
        m_is_unit_stepped = is_unit_stepped;
    }

private:
    DomainDuration m_tau{DEFAULT_TAU, DEFAULT_TAU_TYPE};
    bool m_is_unit_stepped = DEFAULT_UNIT_STEP;

    double m_x_prev = 0.0;
    double m_y = 0.0;
    std::optional<TimePoint> m_t1 = std::nullopt;
};
} // namespace serialist

#endif //SERIALISTLOOPER_FILTERS_H
