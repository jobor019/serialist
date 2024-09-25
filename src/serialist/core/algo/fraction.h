
#ifndef SERIALISTLOOPER_FRACTION_H
#define SERIALISTLOOPER_FRACTION_H

#include <cmath>
#include <chrono>

#include "core/collections/vec.h"

namespace serialist {

class Fraction {
public:
    Fraction(long num, long denom) : n(num), d(denom) {}


    explicit operator double() const {
        return n / static_cast<double>(d);
    }


    long n;
    long d;
};





// ==============================================================================================

/**
 * @brief Fraction with an integral part, a numerator and a denominator
 */
class ExtendedFraction {
public:
    ExtendedFraction(long integral_part, long num, long denom)
            : m_integral_part(integral_part), m_fractional_part(num, denom) {}

    ExtendedFraction(long integral_part, Fraction fractional_part)
            : m_integral_part(integral_part), m_fractional_part(fractional_part) {}


    static ExtendedFraction from_decimal(double y, const Vec<long>& allowed_denoms, double epsilon = 1e-6) {
        long sign = (y < 0.0) ? -1 : 1;
        y = std::abs(y);

        double integral_part = std::floor(y);
        double fractional_part = y - integral_part;

        long best_n = 0;
        long best_d = 1;

        auto min_distance = std::numeric_limits<double>::max();

        for (auto denom: allowed_denoms) {
            long n = std::lround(fractional_part * static_cast<double>(denom));
            double distance = std::abs(static_cast<double>(n) / static_cast<double>(denom) - fractional_part);

            if (distance < min_distance || (utils::equals(distance, min_distance, epsilon) && denom < best_d)) {
                min_distance = distance;
                best_n = n;
                best_d = denom;
            }

            // early exit if we're close enough
            if (utils::equals(distance, 0.0, epsilon)) {
                break;
            }
        }

        return {std::lround(integral_part) * sign, Fraction{sign * best_n, best_d}};
    }


    /**
     * @brief approximate fraction from a decimal using a Stern-Brocot tree
     */
    static ExtendedFraction from_decimal(double y, long max_denominator, double epsilon = 1e-6) {
        long sign = (y < 0.0) ? -1 : 1;
        y = std::abs(y);

        double integral_part = std::floor(y);
        double fractional_part = y - integral_part;

        long a = 0, b = 1;  // Left fraction (a / b)
        long c = 1, d = 1;  // Right fraction (c / d)

        while (b + d <= max_denominator) {
            long mediant_numerator = a + c;
            long mediant_denominator = b + d;
            double mediant = static_cast<double>(mediant_numerator) / static_cast<double>(mediant_denominator);

            if (utils::equals(mediant, fractional_part, epsilon)) {
                return {std::lround(integral_part) * sign, Fraction{sign * mediant_numerator, mediant_denominator}};
            }

            if (fractional_part > mediant) {
                a = mediant_numerator;
                b = mediant_denominator;
            } else {
                c = mediant_numerator;
                d = mediant_denominator;
            }
        }

        double ab_distance = std::abs(static_cast<double>(a) / static_cast<double>(b) - fractional_part);
        double cd_distance = std::abs(static_cast<double>(c) / static_cast<double>(d) - fractional_part);

        if (ab_distance < cd_distance || (utils::equals(ab_distance, cd_distance, epsilon) && b < d)) {
            return {std::lround(integral_part) * sign, Fraction{sign * a, b}};
        } else {
            return {std::lround(integral_part) * sign, Fraction{sign * c, d}};
        }
    }


    long get_integral_part() const { return m_integral_part; }

    const Fraction& get_fractional_part() const { return m_fractional_part; }

    long get_n() const { return m_fractional_part.n; }

    long get_d() const { return m_fractional_part.d; }

    double get_decimal() const {
        return static_cast<double>(m_integral_part) +
               static_cast<double>(m_fractional_part.n) / static_cast<double>(m_fractional_part.d);
    }

private:
    long m_integral_part;
    Fraction m_fractional_part;
};


} // namespace serialist


#endif //SERIALISTLOOPER_FRACTION_H
