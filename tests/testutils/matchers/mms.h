#ifndef TESTUTILS_MMS_H
#define TESTUTILS_MMS_H

#include <catch2/catch_test_macros.hpp>

#include "core/collections/voices.h"
#include "matchers_common.h"
#include "conditions/cms.h"

using namespace serialist;
using namespace serialist::test;


namespace serialist::test::mms {
template<typename T>
ResultMatcher<T> size(std::size_t n, MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {cms::size<T>(n), "has " + serialize(n) + " voices", match_type, allow_no_comparison};
}


/** Match that the output Voices<T> has n monophonic voices*/
inline ResultMatcher<Event> sizee(std::size_t n
                                  , MatchType match_type = MatchType::last
                                  , bool allow_no_comparison = false) {
    return {cms::size<Event>(n), "has " + serialize(n) + " voices", match_type, allow_no_comparison};
}


// ==============================================================================================
// EVENTS
// ==============================================================================================

inline ResultMatcher<Event> voice_equals(std::size_t voice_index
                                         , const Vec<NoteComparator>& cs
                                         , MatchType match_type = MatchType::last
                                         , bool allow_no_comparison = false) {
    return {cms::voice_equals(voice_index, cs)
            , "voice " + serialize(voice_index) + " contains " + cs.as_type<std::string>().to_string()
            , match_type
            , allow_no_comparison
    };
}


inline ResultMatcher<Event> voice_equals(std::size_t voice_index
                                         , const NoteComparator c
                                         , MatchType match_type = MatchType::last
                                         , bool allow_no_comparison = false) {
    return voice_equals(voice_index, Vec{c}, match_type, allow_no_comparison);
}


inline ResultMatcher<Event> voice_contains(std::size_t voice_index
                                         , const Vec<NoteComparator>& cs
                                         , MatchType match_type = MatchType::last
                                         , bool allow_no_comparison = false) {
    return {cms::voice_contains(voice_index, cs)
            , "voice " + serialize(voice_index) + " contains  " + cs.as_type<std::string>().to_string()
            , match_type
            , allow_no_comparison
    };
}

inline ResultMatcher<Event> voice_contains(std::size_t voice_index
                                         , const NoteComparator c
                                         , MatchType match_type = MatchType::last
                                         , bool allow_no_comparison = false) {
    return voice_contains(voice_index, Vec{c}, match_type, allow_no_comparison);
}

}


#endif //TESTUTILS_MMS_H
