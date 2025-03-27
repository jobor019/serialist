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


inline ResultMatcher<Event> emptye(MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {c1m::emptye(), "is empty", match_type, allow_no_comparison};
}


/** Match that the output Voices<T> has a single element of size n */
template<typename T>
ResultMatcher<T> size(std::size_t n, MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {c1m::size<T>(n), "has single vector of size " + serialize(n), match_type, allow_no_comparison};
}


inline ResultMatcher<Trigger> sizet(std::size_t n
                                    , MatchType match_type = MatchType::last
                                    , bool allow_no_comparison = false) {
    return {c1m::sizet(n), "has single vector of size " + serialize(n), match_type, allow_no_comparison};
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

inline ResultMatcher<Trigger> sortedt(MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {c1m::sortedt(), "is sorted", match_type, allow_no_comparison};
}


// ==============================================================================================
// EVENT MATCHERS
// ==============================================================================================

inline ResultMatcher<Event> equals_chord(const Vec<NoteComparator>& cs, MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {c1m::equals_chord(cs), "equals chord " + cs.as_type<std::string>().to_string(), match_type, allow_no_comparison};
}

inline ResultMatcher<Event> contains_note(const NoteComparator& c, MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {c1m::contains_note(c), "contains chord " + static_cast<std::string>(c), match_type, allow_no_comparison};
}

inline ResultMatcher<Event> contains_chord(const Vec<NoteComparator>& cs, MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {c1m::contains_chord(cs), "contains chord " + cs.as_type<std::string>().to_string(), match_type, allow_no_comparison};
}




}

#endif //TEST_UTILS_M1M_H
