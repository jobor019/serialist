
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

    double process(double x, double t0) {
        if (!t1.has_value() || t0 < t1.value()) {
            reset(t0);
            return y1;
        }

        if (tau < TAU_THRESHOLD) {
            y1 = x;
            t1 = t0;
            return y1;
        }

        auto dt = t0 - t1.value();
        t1 = t0;

        auto s = std::exp(-dt / tau);
        y1 = (1 - s) * x + s * y1;
        return y1;
    }


    void reset(double t0) {
        y1 = 0;
        t1 = t0;
    }


    void set_tau(double new_tau) {
        if (new_tau <= 0) {
            throw std::invalid_argument("tau must be positive");
        }
        tau = new_tau;
    }


private:
    double y1 = 0.0;

    double tau = 0.0;   // ticks

    std::optional<double> t1 = std::nullopt;
};

#endif //SERIALISTLOOPER_FILTERS_H
