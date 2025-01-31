#ifndef TESTUTILS_C11_H
#define TESTUTILS_C11_H
#include "condition.h"


namespace serialist::test::c11 {
// ==============================================================================================
// VALUE COMPARISONS
// ==============================================================================================


// TODO: This shouldn't really live in c11
template<typename T>
std::unique_ptr<ValueComparison<T>> custom_comparison(const std::function<bool(const T&)>& f) {
    return std::make_unique<ValueComparison<T>>(f);
}


// TODO: This shouldn't really live in c11
template<typename T>
std::unique_ptr<EmptyComparison<T>> empty() {
    return std::make_unique<EmptyComparison<T>>();
}


inline std::unique_ptr<EmptyComparison<Facet>> emptyf() {
    return std::make_unique<EmptyComparison<Facet>>();
}


// ==============================================================================================

template<typename T, typename Comparator>
std::unique_ptr<ValueComparison<T>> make_value_comparison(const T& expected, Comparator comp) {
    return std::make_unique<ValueComparison<T>>(std::bind(comp, std::placeholders::_1, expected));
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template<typename T>
std::unique_ptr<ValueComparison<T>> eq(const T& expected) {
    return make_value_comparison(expected, std::equal_to<T>());
}


// Note: Facet already implements std::equal_to as a float comparison with EPSILON = 1e-8
template<typename T>
std::unique_ptr<ValueComparison<Facet>> eqf(const T& expected) { return eq(fcast(expected)); }


template<typename T>
std::unique_ptr<ValueComparison<T>> approx_eq(const T& expected, const T& epsilon = static_cast<T>(EPSILON)) {
    assert(epsilon > 0.0);
    return std::make_unique<ValueComparison<T>>([=](const T& v) {
        return utils::equals<T>(v, expected, epsilon);
    });
}


// Note: Only for cases where a custom epsilon is needed. Facet already implements std::equal_to as a float comparison
template<typename T>
std::unique_ptr<ValueComparison<Facet>> approx_eqf(const T& expected, const T& epsilon) {
    return approx_eq(fcast(expected), fcast(epsilon));
}


template<typename T>
std::unique_ptr<ValueComparison<T>> circular_eq(const T& expected
                                                , const T& epsilon = static_cast<T>(EPSILON)
                                                , const T& modulo_range = static_cast<T>(1.0)) {
    assert(epsilon > 0.0);
    assert(modulo_range > 0.0);
    return std::make_unique<ValueComparison<T>>([=](const T& v) {
        return utils::circular_equals<T>(v, expected, epsilon, modulo_range);
    });
}


template<typename T>
std::unique_ptr<ValueComparison<Facet>> circular_eqf(const T& expected
                                                     , const T& epsilon = static_cast<T>(EPSILON)
                                                     , const T& modulo_range = static_cast<T>(1.0)) {
    return circular_eq(fcast(expected), fcast(epsilon), fcast(modulo_range));
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


template<typename T>
std::unique_ptr<ValueComparison<T>> ge(const T& expected) {
    return make_value_comparison(expected, std::greater_equal<T>());
}


template<typename T>
std::unique_ptr<ValueComparison<Facet>> gef(const T& expected) { return ge(fcast(expected)); }


template<typename T>
std::unique_ptr<ValueComparison<T>> gt(const T& expected) {
    return make_value_comparison(expected, std::greater<T>());
}


template<typename T>
std::unique_ptr<ValueComparison<Facet>> gtf(const T& expected) { return gt(fcast(expected)); }


template<typename T>
std::unique_ptr<ValueComparison<T>> le(const T& expected) {
    return make_value_comparison(expected, std::less_equal<T>());
}


template<typename T>
std::unique_ptr<ValueComparison<Facet>> lef(const T& expected) { return le(fcast(expected)); }


template<typename T>
std::unique_ptr<ValueComparison<T>> lt(const T& expected) {
    return make_value_comparison(expected, std::less<T>());
}


template<typename T>
std::unique_ptr<ValueComparison<Facet>> ltf(const T& expected) { return lt(fcast(expected)); }


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template<typename T>
std::unique_ptr<ValueComparison<T>> in_range(const T& low
                                             , const T& high
                                             , bool end_inclusive = false
                                             , bool start_inclusive = true) {
    static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type");

    return std::make_unique<ValueComparison<T>>([=](const T& v) {
        return utils::in<T>(v, low, high, start_inclusive, end_inclusive);
    });
}


template<typename T>
std::unique_ptr<ValueComparison<Facet>> in_rangef(const T& low
                                                  , const T& high
                                                  , bool end_inclusive = false
                                                  , bool start_inclusive = true) {
    return in_range(fcast(low), fcast(high), end_inclusive, start_inclusive);
}


template<typename T>
std::unique_ptr<ValueComparison<Facet>> approx_in_rangef(const T& low
                                                         , const T& high
                                                         , const T& epsilon = EPSILON
                                                         , bool end_inclusive = false
                                                         , bool start_inclusive = true) {
    return in_range<Facet>(fcast(low - epsilon), fcast(high + epsilon), end_inclusive, start_inclusive);
}


// ==============================================================================================
// CHANGE COMPARISONS
// ==============================================================================================

template<typename T, typename Comparator>
std::unique_ptr<ValueChangeComparison<T>> make_change_comparison(Comparator&& comp) {
    return std::make_unique<ValueChangeComparison<T>>(std::move(comp));
}


inline std::unique_ptr<ValueChangeComparison<Facet>> strictly_increasingf() {
    return make_change_comparison<Facet>(std::less<Facet>());
}


inline std::unique_ptr<ValueChangeComparison<Facet>> increasingf() {
    return make_change_comparison<Facet>(std::less_equal<Facet>());
}


inline std::unique_ptr<ValueChangeComparison<Facet>> strictly_decreasingf() {
    return make_change_comparison<Facet>(std::greater<Facet>());
}


inline std::unique_ptr<ValueChangeComparison<Facet>> decreasingf() {
    return make_change_comparison<Facet>(std::greater_equal<Facet>());
}
}
#endif //TESTUTILS_C11_H
