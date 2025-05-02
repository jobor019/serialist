#ifndef TESTUTILS_MMS_H
#define TESTUTILS_MMS_H

#include <catch2/catch_test_macros.hpp>

#include "core/collections/voices.h"
#include "matchers_common.h"
#include "conditions/cms.h"

using namespace serialist;
using namespace serialist::test;


/**
 * polyphonic matchers for Voices<T> of arbitrary sizes (N x M)
 */
namespace serialist::test::mms {

/**
 * @brief Match that the output Voice<T> at index `voice_index` is empty.
 *        For matching entire Voices<T> see `m11::empty`
 */
template<typename T>
ResultMatcher<T> empty(std::size_t voice_index
                       , MatchType match_type = MatchType::last
                       , bool allow_no_comparison = false) {
    return {cms::empty<T>(voice_index), "is empty", match_type, allow_no_comparison};
}


inline ResultMatcher<Facet> emptyf(std::size_t voice_index
                                   , MatchType match_type = MatchType::last
                                   , bool allow_no_comparison = false) {
    return empty<Facet>(voice_index, match_type, allow_no_comparison);
}


inline ResultMatcher<Trigger> emptyt(std::size_t voice_index
                                     , MatchType match_type = MatchType::last
                                     , bool allow_no_comparison = false) {
    return empty<Trigger>(voice_index, match_type, allow_no_comparison);
}


inline ResultMatcher<Event> emptye(std::size_t voice_index
                                   , MatchType match_type = MatchType::last
                                   , bool allow_no_comparison = false) {
    return empty<Event>(voice_index, match_type, allow_no_comparison);
}


template<typename T>
ResultMatcher<T> size(std::size_t n, MatchType match_type = MatchType::last, bool allow_no_comparison = false) {
    return {cms::size<T>(n), "has " + serialize(n) + " voices", match_type, allow_no_comparison};
}

template<typename T>
ResultMatcher<T> size(std::size_t voice_index
                      , std::size_t n
                      , MatchType match_type = MatchType::last
                      , bool allow_no_comparison = false) {
    return {cms::size<T>(n), "has voice " + serialize(voice_index) + " with " + serialize(n) + " entries", match_type, allow_no_comparison};
}

inline ResultMatcher<Facet> sizef(std::size_t n
                                  , MatchType match_type = MatchType::last
                                  , bool allow_no_comparison = false) {
    return size<Facet>(n, match_type, allow_no_comparison);
}


inline ResultMatcher<Trigger> sizet(std::size_t n
                                    , MatchType match_type = MatchType::last
                                    , bool allow_no_comparison = false) {
    return size<Trigger>(n, match_type, allow_no_comparison);
}

inline ResultMatcher<Trigger> sizet(std::size_t voice_index
                                    , std::size_t n
                                    , MatchType match_type = MatchType::last
                                    , bool allow_no_comparison = false) {
    return size<Trigger>(voice_index, n, match_type, allow_no_comparison);
}

/** Match that the output Voices<T> has n voices*/
inline ResultMatcher<Event> sizee(std::size_t n
                                  , MatchType match_type = MatchType::last
                                  , bool allow_no_comparison = false) {
    return {cms::size<Event>(n), "has " + serialize(n) + " voices", match_type, allow_no_comparison};
}

// ==============================================================================================
// FACET MATCHERS
// ==============================================================================================

template<typename T>
ResultMatcher<Facet> eqf(std::size_t voice_index
                         , const Vec<T>& expected
                         , MatchType match_type = MatchType::last
                         , bool allow_no_comparison = false) {
    return {cms::eqf(voice_index, expected)
            , "@voice=" + serialize(voice_index) + "is == " + expected.template as_type<std::string>([](const T& t) {
                return serialize(t);
            }).to_string()
            , match_type
            , allow_no_comparison
    };
}

template<typename T>
ResultMatcher<Facet> eqf(std::size_t voice_index
                         , const T& expected
                         , MatchType match_type = MatchType::last
                         , bool allow_no_comparison = false) {
    return eqf(voice_index, Vec<T>::singular(expected), match_type, allow_no_comparison);

}

// ==============================================================================================
// TRIGGER MATCHERS
// ==============================================================================================

inline ResultMatcher<Trigger> containst(Trigger::Type type
                                        , std::size_t voice_index
                                        , std::optional<std::size_t> id = std::nullopt
                                        , MatchType match_type = MatchType::last
                                        , bool allow_no_comparison = false) {
    return {cms::containst(type, voice_index, id)
            , "@voice=" + serialize(voice_index) + " contains " + Trigger::format(type, id)
            , match_type
            , allow_no_comparison
    };
}


inline ResultMatcher<Trigger> containst_off(std::size_t voice_index
                                             , std::optional<std::size_t> id = std::nullopt
                                             , MatchType match_type = MatchType::last
                                             , bool allow_no_comparison = false) {
    return containst(Trigger::Type::pulse_off, voice_index, id, match_type, allow_no_comparison);
}


inline ResultMatcher<Trigger> containst_on(std::size_t voice_index
                                            , std::optional<std::size_t> id = std::nullopt
                                            , MatchType match_type = MatchType::last
                                            , bool allow_no_comparison = false) {
    return containst(Trigger::Type::pulse_on, voice_index, id, match_type, allow_no_comparison);
}


inline ResultMatcher<Trigger> equalst(Trigger::Type type
                                      , std::size_t voice_index
                                      , std::optional<std::size_t> id = std::nullopt
                                      , MatchType match_type = MatchType::last
                                      , bool allow_no_comparison = false) {
    return {cms::equalst(type, voice_index, id)
            , "@voice=" + serialize(voice_index) + " = " + Trigger::format(type, id)
            , match_type
            , allow_no_comparison
    };
}


inline ResultMatcher<Trigger> equalst(Trigger::Type type
                                      , std::pair<std::size_t, std::size_t>&& voice_and_entry_index
                                      , std::optional<std::size_t> id = std::nullopt
                                      , MatchType match_type = MatchType::last
                                      , bool allow_no_comparison = false) {
    std::string description = "@voice=" + serialize(voice_and_entry_index.first)
                              + ", entry=" + serialize(voice_and_entry_index.second)
                              + " = " + Trigger::format(type, id);
    return {cms::equalst(type, std::move(voice_and_entry_index), id)
            , description
            , match_type
            , allow_no_comparison
    };
}


inline ResultMatcher<Trigger> equalst_on(std::size_t voice_index
                                         , std::optional<std::size_t> id = std::nullopt
                                         , MatchType match_type = MatchType::last
                                         , bool allow_no_comparison = false) {
    return equalst(Trigger::Type::pulse_on, voice_index, id, match_type, allow_no_comparison);
}


inline ResultMatcher<Trigger> equalst_on(std::pair<std::size_t, std::size_t>&& voice_and_entry_index
                                         , std::optional<std::size_t> id = std::nullopt
                                         , MatchType match_type = MatchType::last
                                         , bool allow_no_comparison = false) {
    return equalst(Trigger::Type::pulse_on, std::move(voice_and_entry_index), id, match_type, allow_no_comparison);
}


inline ResultMatcher<Trigger> equalst_off(std::size_t voice_index
                                          , std::optional<std::size_t> id = std::nullopt
                                          , MatchType match_type = MatchType::last
                                          , bool allow_no_comparison = false) {
    return equalst(Trigger::Type::pulse_off, voice_index, id, match_type, allow_no_comparison);
}


inline ResultMatcher<Trigger> equalst_off(std::pair<std::size_t, std::size_t>&& voice_and_entry_index
                                          , std::optional<std::size_t> id = std::nullopt
                                          , MatchType match_type = MatchType::last
                                          , bool allow_no_comparison = false) {
    return equalst(Trigger::Type::pulse_off, std::move(voice_and_entry_index), id, match_type, allow_no_comparison);
}


// ==============================================================================================
// EVENT MATCHERS
// ==============================================================================================

inline ResultMatcher<Event> voice_equals(std::size_t voice_index
                                         , const Vec<NoteComparator>& cs
                                         , MatchType match_type = MatchType::last
                                         , bool allow_no_comparison = false) {
    return {cms::equalse(voice_index, cs)
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
    return {cms::containse(voice_index, cs)
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
