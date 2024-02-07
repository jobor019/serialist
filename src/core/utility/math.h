
#ifndef SERIALISTLOOPER_MATH_H
#define SERIALISTLOOPER_MATH_H

#include <type_traits>
#include <cmath>
#include <stdexcept>
#include <optional>
#include <algorithm>

namespace utils {

/**
 * @brief Computes the remainder of a division between two doubles, allowing negative nominators.
 *
 * This function computes the remainder of the division of a double value `n` by another double value `d`,
 * ensuring that the result is always positive.
 *
 * @param n The dividend.
 * @param d The divisor.
 * @return The remainder of the division `n` by `d`, always positive.
 */
template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
inline T modulo(T n, T d) {
    if constexpr (std::is_integral_v<T>) {
        return ((n % d) + d) % d;
    } else {
        return std::fmod(std::fmod(n, d) + d, d);
    }
}


// ==============================================================================================

template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
inline T clip(T position
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
template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
inline T clip(T position, T lower_bound, T upper_bound) {
    return clip(position, std::optional<T>(lower_bound), std::optional<T>(upper_bound));
}


// ==============================================================================================

template<typename IntType, typename = std::enable_if_t<std::is_integral_v<IntType>>>
inline IntType sign_index(IntType index, std::size_t container_size) {
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

inline bool equals(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}


} // namespace utils

#endif //SERIALISTLOOPER_MATH_H
