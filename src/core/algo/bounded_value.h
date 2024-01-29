
#ifndef SERIALISTLOOPER_BOUNDED_VALUE_H
#define SERIALISTLOOPER_BOUNDED_VALUE_H

#include <optional>
#include "core/utility/math.h"

template<typename T>
class Quantizer {
public:
    static Quantizer from_step_size(T step_size) {

    }

    static Quantizer from_num_steps(unsigned int num_steps) {

    }


private:
    Quantizer(unsigned int num_steps, T step_size)
        : m_num_steps(num_steps), m_step_size(step_size) {
        static_assert(std::is_arithmetic_v<T>);
    }

    static T step_size_of(unsigned int num_steps) {
        if (num_steps)
    }

    unsigned int m_num_steps;
    T m_step_size;
};

template<typename T>
class Exponential {
public:
    explicit Exponential(double exponent) : m_exponent(exponent) {
        static_assert(std::is_arithmetic_v<T>);
    }

    T apply(double raw_value) const {
        return static_cast<T>(std::pow(raw_value, m_exponent));
    }

    double inverse(T output_value) const {
        if (utils::equals(m_exponent, 0.0, 1e-9))
            return static_cast<double>(output_value);

        return std::pow(static_cast<double>(output_value), 1.0 / m_exponent);
    }


private:
    double m_exponent;
};

template<typename T>
class Bounds {
    public:
        explicit Bounds(T lower_bound = 0.0
                        , T upper_bound = 1.0
                        , std::optional<T> lower_hard_bound = std::nullopt
                        , std::optional<T> upper_hard_bound = std::nullopt)
                : m_lower_bound(lower_bound)
                  , m_upper_bound(upper_bound)
                  , m_lower_hard_bound(lower_hard_bound)
                  , m_upper_hard_bound(upper_hard_bound) {
            static_assert(std::is_arithmetic_v<T>);
        }


        T get_lower_bound() const { return m_lower_bound; }

        T get_upper_bound() const { return m_upper_bound; }

        std::optional<T> get_lower_hard_bound() const { return m_lower_hard_bound; }

        std::optional<T> get_upper_hard_bound() const { return m_upper_hard_bound; }

        T clip(T v) const { return utils::clip(v, {m_lower_bound}, {m_upper_bound}); }

        T apply(double raw_value) const {
            assert (raw_value >= 0.0 && raw_value <= 1.0);

            auto range = m_upper_bound - m_lower_bound;

            if (range <= static_cast<T>(0))
                return static_cast<T>(0);

            auto scaled_value = utils::clip(raw_value * range + m_lower_bound, {m_lower_bound}, {m_upper_bound});

            return scaled_value;
        }

        double inverse(T output_value) const {
            auto range = m_upper_bound - m_lower_bound;

            if (range <= static_cast<T>(0))
                return static_cast<double>(m_lower_bound);

            return utils::clip(static_cast<double>((output_value - m_lower_bound) / range), {0.0}, {1.0});
        }

    private:
        void set_lower_bound(T v) { m_lower_bound = utils::clip(v, m_lower_hard_bound, m_upper_hard_bound); }

        void set_upper_bound(T v) { m_lower_bound = utils::clip(v, m_lower_hard_bound, m_upper_hard_bound); }


        T m_lower_bound;
        T m_upper_bound;

        std::optional<T> m_lower_hard_bound;
        std::optional<T> m_upper_hard_bound;



    };


template<typename T>
class BoundedValue {
public:
private:
    double m_raw_value;
        double m_scaled_value;

};

#endif //SERIALISTLOOPER_BOUNDED_VALUE_H
