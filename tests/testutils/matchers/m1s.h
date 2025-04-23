
#ifndef TESTUTILS_M2S_H
#define TESTUTILS_M2S_H


#include <catch2/catch_test_macros.hpp>

#include "core/collections/voices.h"
#include "matchers_common.h"
#include "conditions/c1s.h"

using namespace serialist;
using namespace serialist::test;

/**
 * Sequential matchers for Voices<T> of size (S x 1), i.e. voices.size() == S and voices[s].size() == 1 for all s in S
 */
namespace serialist::test::m1s {

/** Match that the output Voices<T> has n monophonic voices*/
template<typename T>
ResultMatcher<T> size(std::size_t n, MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {c1s::size<T>(n), "has " + serialize(n) + " monophonic voices", match_type, allow_no_comparison};
}


/** Match that the output Voices<T> has n monophonic voices*/
inline ResultMatcher<Event> sizee(std::size_t n, MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {c1s::size<Event>(n), "has " + serialize(n) + " monophonic voices", match_type, allow_no_comparison};
}


// ==============================================================================================

template<typename T>
ResultMatcher<T> eq(const Vec<T>& expected, MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {c1s::eq(expected), "is == " + expected.template as_type<std::string>().to_string(), match_type, allow_no_comparison};
}

template<typename T, typename... Args>
ResultMatcher<Facet> eqf(const Vec<T>& expected, Args&&... args) {
    return eq(expected.template as_type<Facet>(), std::forward<Args>(args)...);
}


// ==============================================================================================
// EVENTS
// ==============================================================================================

inline ResultMatcher<Event> equals_sequence(const Vec<NoteComparator>& cs, MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {c1s::equals_sequence(cs), "equals sequence " + cs.as_type<std::string>().to_string(), match_type, allow_no_comparison};
}


}
#endif //TESTUTILS_M2S_H
