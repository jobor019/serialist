#ifndef TEST_UTILS_M1M_H
#define TEST_UTILS_M1M_H

#include <catch2/catch_test_macros.hpp>

#include "core/collections/voices.h"
#include "matchers_common.h"
#include "conditions/c1m.h"

using namespace serialist;
using namespace serialist::test;


namespace serialist::test::m1m {
template<typename T>
ResultMatcher<T> empty(MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {c1m::empty<T>(), "is empty", match_type, allow_no_comparison};
}


inline ResultMatcher<Trigger> emptyt(MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {c1m::emptyt(), "is empty", match_type, allow_no_comparison};
}


template<typename T>
ResultMatcher<T> size(std::size_t n, MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {c1m::size<T>(n), "has size " + serialize(n), match_type, allow_no_comparison};
}


inline ResultMatcher<Trigger> sizet(std::size_t n
                                    , MatchType match_type = MatchType::last
                                    , bool allow_no_comparison = false) {
    return {c1m::sizet(n), "has size " + serialize(n), match_type, allow_no_comparison};
}


inline ResultMatcher<Trigger> containst(Trigger::Type type
                                        , std::optional<std::size_t> id = std::nullopt
                                        , MatchType match_type = MatchType::last
                                        , bool allow_no_comparison = false) {
    return {c1m::containst(type, id), "contains " + Trigger::format(type, id), match_type, allow_no_comparison};
}


inline ResultMatcher<Trigger> containst_off(std::optional<std::size_t> id = std::nullopt
                                            , MatchType match_type = MatchType::last
                                            , bool allow_no_comparison = false) {
    return {c1m::containst_off(id)
            , "contains " + Trigger::format(Trigger::Type::pulse_off, id)
            , match_type
            , allow_no_comparison
    };
}


inline ResultMatcher<Trigger> containst_on(std::optional<std::size_t> id = std::nullopt
                                           , MatchType match_type = MatchType::last
                                           , bool allow_no_comparison = false) {
    return {c1m::containst_on(id)
            , "contains " + Trigger::format(Trigger::Type::pulse_on, id)
            , match_type
            , allow_no_comparison
    };
}


inline ResultMatcher<Trigger> equalst(Trigger::Type type
                                      , std::optional<std::size_t> id = std::nullopt
                                      , MatchType match_type = MatchType::last
                                      , bool allow_no_comparison = false) {
    return {c1m::equalst(type, id), "= " + Trigger::format(type, id), match_type, allow_no_comparison};
}


inline ResultMatcher<Trigger> equalst_off(std::optional<std::size_t> id = std::nullopt
                                          , MatchType match_type = MatchType::last
                                          , bool allow_no_comparison = false) {
    return {c1m::equalst_off(id)
            , "= " + Trigger::format(Trigger::Type::pulse_off, id)
            , match_type
            , allow_no_comparison
    };
}


inline ResultMatcher<Trigger> equalst_on(std::optional<std::size_t> id = std::nullopt
                                         , MatchType match_type = MatchType::last
                                         , bool allow_no_comparison = false) {
    return {c1m::equalst_on(id)
            , "= " + Trigger::format(Trigger::Type::pulse_on, id)
            , match_type
            , allow_no_comparison
    };
}
}

#endif //TEST_UTILS_M1M_H
