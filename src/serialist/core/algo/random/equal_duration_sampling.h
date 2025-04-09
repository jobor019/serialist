
#ifndef SERIALISTLOOPER_EQUAL_DURATION_SAMPLING_H
#define SERIALISTLOOPER_EQUAL_DURATION_SAMPLING_H

#include <cassert>
#include "core/algo/random/random.h"

namespace serialist {

/**
 * Randomly samples durations in interval [lower_bound, upper_bound) using an inverse exponential distribution
 * See Obsidian docs for details and equations
 */
class EqualDurationSampling {
public:
    EqualDurationSampling(double lower_bound, double upper_bound, std::optional<unsigned int> seed = std::nullopt)
            : m_lower_bound(lower_bound)
              , m_upper_bound(upper_bound)
              , m_random(seed) {
        assert(lower_bound <= upper_bound);
        recompute_coefficients();
    }

    double next(double uniform_rand) const {
        return inverse_cdf(uniform_rand);
    }

    double next() {
        return inverse_cdf(m_random.next());
    }


    void set_lower_bound(double lower_bound, bool recompute = true) {
        m_lower_bound = lower_bound;
        if (recompute)
            recompute_coefficients();
    }


    void set_upper_bound(double upper_bound, bool recompute = true) {
        m_upper_bound = upper_bound;
        if (recompute)
            recompute_coefficients();
    }


    void set_bounds(double lower_bound, double upper_bound, bool recompute = true) {
        m_lower_bound = lower_bound;
        m_upper_bound = upper_bound;
        if (recompute)
            recompute_coefficients();
    }


    void recompute_coefficients() {
        auto q = std::pow(0.5, 1 / m_lower_bound);
        m_log_q = std::log(q);
        auto gamma = m_log_q / (2 * (std::pow(q, m_upper_bound) - 0.5));

        m_k_inv = m_log_q / (2 * gamma);
    }


    double pdf(double x) const {
        if (x < m_lower_bound || x > m_upper_bound)
            return 0.0;

        return 2.0 * std::pow(0.5, x / m_lower_bound);
    }


    double inverse_cdf(double u) const {
        if (u < 0.0 || u >= 1.0) {
            throw std::invalid_argument("Value must be in (0, 1)");
        }

        return 1 / m_log_q * std::log(u * m_k_inv + 0.5);
    }


    double get_lower_bound() const { return m_lower_bound; }
    double get_upper_bound() const { return m_upper_bound; }

private:


    double m_lower_bound;
    double m_upper_bound;

    Random m_random;

    double m_log_q = 0.0;
    double m_k_inv = 0.0;
};

} // namespace serialist

#endif //SERIALISTLOOPER_EQUAL_DURATION_SAMPLING_H
