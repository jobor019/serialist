#ifndef V11_H
#define V11_H

#include <catch2/catch_test_macros.hpp>

#include "core/collections/voices.h"
#include "matchers_common.h"
#include "conditions/c11.h"

using namespace serialist;
using namespace serialist::test;


namespace serialist::test::v11 {
template<typename T>
ResultMatcher<T> eq(const T& expected, MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {c11::eq(expected), "is == " + serialize(expected), match_type, allow_no_comparison};
}


template<typename T, typename... Args>
ResultMatcher<Facet> eqf(const T& expected, Args&&... args) {
    return eq(fcast(expected), std::forward<Args>(args)...);
}


template<typename T>
ResultMatcher<T> approx_eq(const T& expected, MatchType match_type = MatchType::last,
                           bool allow_no_comparison = false) {
    return {c11::approx_eq(expected), "is approximately == " + serialize(expected), match_type, allow_no_comparison};
}


template<typename T, typename... Args>
ResultMatcher<Facet> approx_eqf(const T& expected, Args&&... args) {
    return approx_eq(fcast(expected), std::forward<Args>(args)...);
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template<typename T>
ResultMatcher<T> ge(const T& expected, MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {c11::ge(expected), "is >= " + serialize(expected), match_type, allow_no_comparison};
}


template<typename T, typename... Args>
ResultMatcher<Facet> gef(const T& expected, Args&&... args) {
    return ge(fcast(expected), std::forward<Args>(args)...);
}


template<typename T>
ResultMatcher<T> gt(const T& expected, MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {c11::gt(expected), "is > " + serialize(expected), match_type, allow_no_comparison};
}


template<typename T, typename... Args>
ResultMatcher<Facet> gtf(const T& expected, Args&&... args) {
    return gt(fcast(expected), std::forward<Args>(args)...);
}


template<typename T>
ResultMatcher<T> le(const T& expected, MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {c11::le(expected), "is <= " + serialize(expected), match_type, allow_no_comparison};
}


template<typename T, typename... Args>
ResultMatcher<Facet> lef(const T& expected, Args&&... args) {
    return le(fcast(expected), std::forward<Args>(args)...);
}


template<typename T>
ResultMatcher<T> lt(const T& expected, MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {c11::lt(expected), "is < " + serialize(expected), match_type, allow_no_comparison};
}


template<typename T, typename... Args>
ResultMatcher<Facet> ltf(const T& expected, Args&&... args) {
    return lt(fcast(expected), std::forward<Args>(args)...);
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template<typename T>
ResultMatcher<T> in_range(const T& low, const T& high
                          , MatchType match_type = MatchType::last, bool allow_no_comparison = false
                          , bool end_inclusive = false, bool start_inclusive = true) {
    std::stringstream ss;
    ss << "is in range " << (start_inclusive ? "[" : "(")
            << serialize(low) << ", " << serialize(high)
            << (end_inclusive ? "]" : ")");

    return {c11::in_range(low, high, end_inclusive, start_inclusive), ss.str(), match_type, allow_no_comparison};
}


template<typename T>
ResultMatcher<Facet> in_rangef(const T& low, const T& high
                               , MatchType match_type = MatchType::last, bool allow_no_comparison = false
                               , bool end_inclusive = false, bool start_inclusive = true) {
    return in_range(fcast(low), fcast(high), match_type, allow_no_comparison, end_inclusive, start_inclusive);
}


template<typename T>
ResultMatcher<Facet> approx_in_rangef(const T& low, const T& high
                                      , MatchType match_type = MatchType::last, bool allow_no_comparison = false
                                      , const T& epsilon = EPSILON
                                      , bool end_inclusive = false, bool start_inclusive = true) {
    std::stringstream ss;
    ss << "is approximately in range " << (start_inclusive ? "[" : "(")
            << serialize(low) << ", " << serialize(high)
            << (end_inclusive ? "]" : ")") << " with epsilon " << serialize(epsilon);

    return {c11::approx_in_rangef(low, high, epsilon), ss.str(), match_type, allow_no_comparison};
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

inline ResultMatcher<Facet> strictly_increasingf(MatchType match_type = MatchType::all,
                                                 bool allow_no_comparison = false) {
    return {c11::strictly_increasingf(), "is strictly increasing", match_type, allow_no_comparison};
}


inline ResultMatcher<Facet> increasingf(MatchType match_type = MatchType::all, bool allow_no_comparison = false) {
    return {c11::increasingf(), "is increasing", match_type, allow_no_comparison};
}


inline ResultMatcher<Facet> strictly_decreasingf(MatchType match_type = MatchType::all,
                                                 bool allow_no_comparison = false) {
    return {c11::strictly_decreasingf(), "is strictly decreasing", match_type, allow_no_comparison};
}


inline ResultMatcher<Facet> decreasingf(MatchType match_type = MatchType::all, bool allow_no_comparison = false) {
    return {c11::decreasingf(), "is decreasing", match_type, allow_no_comparison};
}
}


#endif //V11_H
