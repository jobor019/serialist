
#ifndef SERIALIST_MAPPING_H
#define SERIALIST_MAPPING_H

#include <type_traits>
#include <cmath>
#include <algorithm>

namespace serialist::utils {

/**
 * @brief Maps a double `x` in range [0., 1.] to a value in range [a, b]
 */
template<typename T = double, typename = std::enable_if_t<std::is_floating_point_v<T>>>
inline double map_exponential(T x, T a, T b, T k) {
    return a + (b - a) * std::pow(x, k);
}

template<typename T = double, typename = std::enable_if_t<std::is_floating_point_v<T>>>
inline double map_exponential(T x, T a, T b, T midpoint, T k1, T k2) {
    if (x < 0.5) {
        return midpoint + (a - midpoint) * std::pow((0.5 - x) / 0.5, k1);
    }
    return midpoint + (b - midpoint) * std::pow((x - 0.5) / 0.5, k2);
}


// ==============================================================================================

template<typename T = double, typename = std::enable_if_t<std::is_floating_point_v<T>>>
inline double quantize(T x, std::size_t num_steps) {
    auto n = static_cast<T>(num_steps);
    return std::min(static_cast<T>(1.0), std::floor(x * n) / (n - 1));
}

} // namespace serialist::utils

#endif //SERIALIST_MAPPING_H
