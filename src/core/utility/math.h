
#ifndef SERIALISTLOOPER_MATH_H
#define SERIALISTLOOPER_MATH_H

#include <type_traits>
#include <cmath>
#include <stdexcept>

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


} // namespace utils

#endif //SERIALISTLOOPER_MATH_H
