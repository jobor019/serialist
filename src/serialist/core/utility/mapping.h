
#ifndef SERIALIST_MAPPING_H
#define SERIALIST_MAPPING_H

#include <type_traits>
#include <cmath>
#include <algorithm>
#include "math.h"

namespace serialist::utils {

/**
 * @brief Maps a double `x` in range [0., 1.] to a value in range [a, b]
 */
template<typename T = double, typename = std::enable_if_t<std::is_floating_point_v<T>>>
inline double map_exponential(T x, T a, T b, T k) {
    assert(k >= 0);
    assert(x >= 0 && x <= 1);
    assert(a <= b);

    return a + (b - a) * std::pow(x, k);
}

template<typename T = double, typename = std::enable_if_t<std::is_floating_point_v<T>>>
inline double map_exponential(T x, T a, T b, T midpoint, T k1, T k2) {
    assert(k1 >= 0);
    assert(k2 >= 0);
    assert(x >= 0 && x <= 1);
    assert(a <= b);
    assert(midpoint >= a && midpoint <= b);

    if (x < 0.5) {
        return midpoint + (a - midpoint) * std::pow((0.5 - x) / 0.5, k1);
    }
    return midpoint + (b - midpoint) * std::pow((x - 0.5) / 0.5, k2);
}


template<typename T = double, typename = std::enable_if_t<std::is_floating_point_v<T>>>
inline double map_inverse_exponential(T y, T a, T b, T k) {
    assert(k >= 0);
    assert(a <= b);
    assert(y >= a && y <= b);

    if (utils::equals(a, b) || utils::equals(k, 0.0)) {
        return 0;
    }
    return std::pow((y - a) / (b - a), 1.0 / k);
}


template<typename T = double, typename = std::enable_if_t<std::is_floating_point_v<T>>>
inline double map_inverse_exponential(T y, T a, T b, T midpoint, T k1, T k2) {
    assert(k1 >= 0);
    assert(k2 >= 0);
    assert(a <= b);
    assert(y >= a && y <= b);
    assert(midpoint >= a && midpoint <= b);

    if (utils::equals(a, b) || utils::equals(k1, 0.0)) {
        return 0;
    }

    if (utils::equals(k2, 0.0)) {
        return 1;
    }

    if (y < midpoint) {
        return 0.5 - 0.5 * std::pow((y - midpoint) / (a - midpoint), 1.0 / k1);
    }

    return 0.5 + 0.5 * std::pow((y - midpoint) / (b - midpoint), 1.0 / k2);
}


// ==============================================================================================

template<typename T = double, typename = std::enable_if_t<std::is_floating_point_v<T>>>
inline double quantize(T x, std::size_t num_steps) {
    auto n = static_cast<T>(num_steps);
    return std::min(static_cast<T>(1.0), std::floor(x * n) / (n - 1));
}

} // namespace serialist::utils

#endif //SERIALIST_MAPPING_H
