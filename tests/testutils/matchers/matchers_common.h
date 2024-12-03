
#ifndef MATCHERS_COMMON_H
#define MATCHERS_COMMON_H
#include <functional>


namespace serialist::test {

// ==============================================================================================
// CONSTANTS
// ==============================================================================================


static constexpr double EPSILON = 1e-8;


// ==============================================================================================
// SIZE CHECKS
// ==============================================================================================

template<typename T>
bool is_empty(const Voices<T>& v) { return v.is_empty_like(); }

template<typename T>
bool is_singular(const Voices<T>& v) { return v.size() == 1 && v[0].size() == 1; }

// Note: "maybe" indicate that the value is either singular or empty
template<typename T>
bool is_maybe_singular(const Voices<T>& v) { return is_empty(v) || is_singular(v); }


// ==============================================================================================
// MISC
// ==============================================================================================


template<typename T>
using Condition = std::function<bool(const T&)>;

template<typename T>
using CompareFunc = std::function<bool(const T&, const T&)>;


}

#endif //MATCHERS_COMMON_H
