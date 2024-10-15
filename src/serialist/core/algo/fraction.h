
#ifndef SERIALISTLOOPER_FRACTION_H
#define SERIALISTLOOPER_FRACTION_H

#include <cmath>
#include <chrono>
#include <sstream>

#include "core/collections/vec.h"

namespace serialist {

class Fraction {
public:

    Fraction() : Fraction(0, 1) {}

    Fraction(long num, long denom) : n(num), d(denom) {
        assert(d > 0);
    }


    explicit operator double() const {
        return static_cast<double>(n) / static_cast<double>(d);
    }

    bool operator>(const Fraction& other) const { return n * other.d > d * other.n; }

    bool operator<(const Fraction& other) const { return n * other.d < d * other.n; }

    bool operator>=(const Fraction& other) const { return n * other.d >= d * other.n; }

    bool operator<=(const Fraction& other) const { return n * other.d <= d * other.n; }


    bool operator>(const double& other) const { return static_cast<double>(n) > other * static_cast<double>(d); }

    bool operator<(const double& other) const { return static_cast<double>(n) < other * static_cast<double>(d); }

    bool operator>=(const double& other) const { return static_cast<double>(n) >= other * static_cast<double>(d); }

    bool operator<=(const double& other) const { return static_cast<double>(n) <= other * static_cast<double>(d); }

    long sign() const { return utils::sign(n); }


    long n;
    long d;
};





// ==============================================================================================

/**
 * @brief Fraction with an integral part, a numerator and a denominator
 */
class ExtendedFraction {
public:
    ExtendedFraction() : ExtendedFraction(0, 1, 1) {}

    ExtendedFraction(long integral_part, long num, long denom)
            : ExtendedFraction(integral_part, Fraction(num, denom)) {}

    ExtendedFraction(long integral_part, Fraction fractional_part)
            : m_integral_part(integral_part), m_fractional_part(fractional_part) {
        assert(utils::sign(m_integral_part) == m_fractional_part.sign()
               || m_integral_part == 0 || m_fractional_part.n == 0);
    }


    static ExtendedFraction from_decimal(double y, const Vec<long>& allowed_denoms, double epsilon = 1e-6) {
        long sign = utils::sign(y);
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

        return normalized(integral_part, sign, best_n, best_d, allowed_denoms);
    }


    /**
     * @brief approximate fraction from a decimal using a Stern-Brocot tree
     */
    static ExtendedFraction from_decimal(double y, long max_denominator, double epsilon = 1e-6) {
        long sign = utils::sign(y);
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
            return normalized(integral_part, sign, a, b, max_denominator);
        } else {
            return normalized(integral_part, sign, c, d, max_denominator);
        }
    }

    enum class Format {
        improper, mixed, list
    };

    std::string to_string(Format fmt = Format::mixed
                          , bool always_include_sign = false
                          , const std::string& sep = " ") const {
        switch (fmt) {
            case Format::improper:
                return format_improper_fraction(always_include_sign);
            case Format::mixed:
                return format_mixed_number(always_include_sign, sep);
            case Format::list:
                return format_fraction_list(sep);
        }
    }


    bool has_fractional_part() const { return m_fractional_part.n != 0; }


    bool has_integral_part() const { return m_integral_part != 0; }


    bool is_zero() const { return !has_fractional_part() && !has_integral_part(); }


    long get_integral_part() const { return m_integral_part; }


    const Fraction& get_fractional_part() const { return m_fractional_part; }


    long get_n() const { return m_fractional_part.n; }


    long get_d() const { return m_fractional_part.d; }


    double get_decimal() const {
        return static_cast<double>(m_integral_part) +
               static_cast<double>(m_fractional_part.n) / static_cast<double>(m_fractional_part.d);
    }


    long sign() const {
        if (m_integral_part == 0) {
            return m_fractional_part.sign();
        }
        return utils::sign(m_integral_part);
    }


private:
    static ExtendedFraction normalized(double integral_part, long sign, long n, long d, long min_allowed_denom) {
        return normalized(integral_part, sign, n, d, Vec<long>{min_allowed_denom});
    }


    static ExtendedFraction normalized(double integral_part, long sign, long n, long d
                                       , const Vec<long>& allowed_denoms) {
        long i = std::lround(integral_part) * sign;
        if (n == d) {
            auto q = Fraction{0, allowed_denoms.min()};
            i += sign;
            return {i, q};
        }
        return {i, Fraction{sign * n, d}};
    }


    std::string format_mixed_number(bool always_include_sign = false, const std::string& sep = " ") const {
        bool fmt_integral_part = has_integral_part();
        bool fmt_fractional_part = has_fractional_part() || std::abs(m_integral_part) > 1;
        long sgn = sign();

        std::stringstream ss;

        if (sgn < 0 || always_include_sign)
            ss << (sgn > 0 ? "+" : "-");


        if (fmt_integral_part)
            ss << std::abs(m_integral_part);

        if (fmt_integral_part && fmt_fractional_part)
            ss << sep;

        if (fmt_fractional_part)
            ss << std::abs(get_n()) << "/" << get_d();

        if (!fmt_integral_part && !fmt_fractional_part)
            ss << 0;

        return ss.str();
    }


    std::string format_improper_fraction(bool always_include_sign = false) const {
        std::stringstream ss;

        long sgn = sign();

        if (sgn < 0 || always_include_sign)
            ss << (sgn > 0 ? "+" : "-");

        ss << std::abs(get_integral_part()) * get_d() + std::abs(get_n()) << "/" << get_d();
        return ss.str();
    }


    std::string format_fraction_list(const std::string& sep = " ") const {
        std::stringstream ss;

        ss << get_integral_part() << sep << get_n() << sep << get_d();
        return ss.str();
    }


    long m_integral_part;
    Fraction m_fractional_part;
};


} // namespace serialist


#endif //SERIALISTLOOPER_FRACTION_H
