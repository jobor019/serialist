
#ifndef SERIALISTLOOPER_FILTERS_H
#define SERIALISTLOOPER_FILTERS_H

#include <cmath>
#include <optional>

/**
 * @brief one-pole smoothing filter for unevenly sampled data
 *
 */
class Smoo {
public:

    static constexpr double TAU_THRESHOLD = 1e-6;

    explicit Smoo(double tau = 0.0) : m_tau(tau) {}

    double process(double x, double t0, bool stepped = false) {
        return process(x, t0, m_tau, stepped);
    }

    double process(double x, double t0, double tau, bool stepped) {
        if (!t1.has_value() || t0 < *t1) {
            reset(t0);
            return y1;
        }

        if (tau < TAU_THRESHOLD) {
            y1 = x;
            t1 = t0;
            return y1;
        }

        auto dt = stepped ? 1.0 : t0 - *t1;
        t1 = t0;

        auto s = std::exp(-dt / tau);
        y1 = (1 - s) * x + s * y1;
        return y1;
    }


    void reset(double t0) {
        y1 = 0;
        t1 = t0;
    }


    void set_tau(double tau) {
        if (tau <= 0) {
            throw std::invalid_argument("tau must be positive");
        }
        m_tau = tau;
    }


private:
    double y1 = 0.0;

    double m_tau;   // ticks

    std::optional<double> t1 = std::nullopt;
};

#endif //SERIALISTLOOPER_FILTERS_H
