
#ifndef SERIALISTLOOPER_EXPONENTIAL_H
#define SERIALISTLOOPER_EXPONENTIAL_H

#include <optional>
#include "core/utility/math.h"

namespace serialist {

template<typename T>
class Exponential {
public:
    static constexpr double DEFAULT_EPSILON = 1e-8;

    explicit Exponential(double exponent, double epsilon = DEFAULT_EPSILON)
    : m_exponent(exponent), m_epsilon(epsilon) {
        static_assert(std::is_arithmetic_v<T>);
    }

    static Exponential<T> deserialize(std::string) {
        throw std::runtime_error("Not implemented: Exponential::deserialize");
    }

    std::string serialize() const {
        throw std::runtime_error("Not implemented: Exponential::serialize");
    }

    T apply(double raw_value) const {
        if (utils::equals(m_exponent, 1.0, m_epsilon))
            return static_cast<T>(raw_value);

        return static_cast<T>(std::pow(raw_value, m_exponent));
    }

    double inverse(T output_value) const {
        if (utils::equals(m_exponent, 0.0, m_epsilon))
            return static_cast<double>(output_value);

        if (utils::equals(m_exponent, 1.0, m_epsilon))
            return static_cast<double>(output_value);

        return std::pow(static_cast<double>(output_value), 1.0 / m_exponent);
    }

    double get_exponent() const { return m_exponent; }
    void set_exponent(double v) { m_exponent = v; }


private:
    double m_exponent;
    double m_epsilon;
};

} // namespace serialist

#endif //SERIALISTLOOPER_EXPONENTIAL_H
