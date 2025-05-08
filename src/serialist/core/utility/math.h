
#ifndef SERIALISTLOOPER_MATH_H
#define SERIALISTLOOPER_MATH_H

#include <type_traits>
#include <cmath>
#include <stdexcept>
#include <optional>
#include <algorithm>

namespace serialist::utils {

/**
 * @brief Computes the modulo of two values, allowing negative nominators, ensuring that the result always is positive
 *
 * @param n The dividend.
 * @param d The divisor.
 * @return The modulo of `n` by `d`, always positive.
 */
template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
T modulo(T n, T d) {
    if constexpr (std::is_integral_v<T>) {
        return ((n % d) + d) % d;
    } else {
        if (n >= 0.0 && n < d) {
            return n; // avoid rounding errors on unit range values
        }
        return std::fmod(std::fmod(n, d) + d, d);
    }
}

template<typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
T modulo(T n, T d, T epsilon) {
    auto r = modulo(n, d);
    if (std::abs(r - d) < epsilon) {
        return 0.0;
    }
    return r;
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
std::pair<T, T> divmod(T n, T d) {
    T remainder = modulo(n, d);
    T quotient = (n - remainder) / d;
    return std::make_pair(quotient, remainder);
}

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
std::pair<T, T> divmod(T n, T d, T epsilon) {
    T remainder = modulo(n, d, epsilon);
    T quotient = (n - remainder) / d;
    return std::make_pair(quotient, remainder);
}


// ==============================================================================================

/**
 * @brief Computes integral floor division, rather than truncation division. Useful for negative numbers, e.g.
 *        floor_division(3, 5)  =  0    (same as 3/5)
 *        floor_division(-3, 5) = -1    (where -3/5 = 0)
 *
 */
template<typename T, typename = std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>>>
T floor_division(T n, T d) {
    T q = n / d;
    // Note: n ^ d for signed types evaluates to whether n and d have different signs
    if ((n ^ d) < 0 && n % d != 0)
        --q;
    return q;
}


// ==============================================================================================

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
T clip(T position
              , std::optional<T> lower_bound = std::nullopt
              , std::optional<T> upper_bound = std::nullopt) {
    if (lower_bound.has_value()) {
        position = std::max(*lower_bound, position);
    }

    if (upper_bound.has_value()) {
        position = std::min(*upper_bound, position);
    }

    return position;
}

// Overload for cases where both lower and upper bounds are provided directly
template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
T clip(T position, T lower_bound, T upper_bound) {
    return clip(position, std::optional<T>(lower_bound), std::optional<T>(upper_bound));
}


// ==============================================================================================

template<typename IntType, typename = std::enable_if_t<std::is_integral_v<IntType>>>
IntType sign_index(IntType index, std::size_t container_size) {
    if (index < 0) {
        return static_cast<IntType>(container_size) + index;
    }
    return index;
}


inline std::size_t double2index(double d, std::size_t index_range, double epsilon = 1e-6) {
    return static_cast<std::size_t>(std::floor(utils::modulo(d + epsilon, 1.0) * static_cast<double>(index_range)));
}


inline double index2double(std::size_t index, std::size_t index_range) {
    return static_cast<double>(index) / static_cast<double>(index_range);
}


// ==============================================================================================

template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
T increment(T value, T fold_value = std::numeric_limits<T>::min()) {
    return (value == std::numeric_limits<T>::max()) ? fold_value : value + 1;
}

template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
T decrement(T value, T fold_value = std::numeric_limits<T>::max()) {
    return (value == std::numeric_limits<T>::min()) ? fold_value : value - 1;
}


// ==============================================================================================

template<typename T = double, typename = std::enable_if_t<std::is_floating_point_v<T>>>
bool equals(T a, T b, T epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

template<typename T = double, typename = std::enable_if_t<std::is_floating_point_v<T>>>
bool equals(std::optional<T> a, std::optional<T> b, T epsilon = 1e-6) {
    if (!a.has_value() || !b.has_value()) {
        return !a.has_value() && !b.has_value();
    }
    return equals(*a, *b, epsilon);
}

template<typename T = double, typename = std::enable_if_t<std::is_floating_point_v<T>>>
bool circular_equals(T a, T b, T epsilon = 1e-6, T modulo_range = static_cast<T>(1.0)) {
    assert(epsilon > 0.0);
    assert(modulo_range > 0.0);

    auto diff = std::abs(a - b);
    if (diff > modulo_range / static_cast<T>(2)) {
        diff = modulo_range - diff;
    }
    return diff < epsilon;
}


// ==============================================================================================

template<typename T = double, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
bool in(T value, T start, T end, bool start_inclusive = true, bool end_inclusive = false) {
    return (start_inclusive ? value >= start : value > start) && (end_inclusive ? value <= end : value < end);
}

// ==============================================================================================

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
long sign(T value) {
    return value < 0 ? -1 : 1;
}


template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
std::size_t num_digits(T num) {
    if (num == 0)
        return 1;
    if (num < 0)
        return static_cast<std::size_t>(std::log10(std::abs(num)) + 2);
    else
        return static_cast<std::size_t>(std::log10(std::abs(num)) + 1);
}


} // namespace serialist::utils

#endif //SERIALISTLOOPER_MATH_H
