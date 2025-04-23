#ifndef TESTUTILS_C1M_H
#define TESTUTILS_C1M_H

#include <memory>

#include "condition.h"
#include "core/types/trigger.h"
#include "c11.h"


/**
 * Chordal conditions for Voices<T> of size (1 x M), i.e. voices.size() == 1 and voices[0].size() == M
 */
namespace serialist::test::c1m {

// ==============================================================================================
// SIZE COMPARISON
// ==============================================================================================

template<typename T>
std::unique_ptr<EmptyComparison<T>> empty() {
    return std::make_unique<EmptyComparison<T>>();
}


inline std::unique_ptr<EmptyComparison<Facet>> emptyf() {
    return std::make_unique<EmptyComparison<Facet>>();
}


inline std::unique_ptr<EmptyComparison<Trigger>> emptyt() {
    return std::make_unique<EmptyComparison<Trigger>>();
}


inline std::unique_ptr<EmptyComparison<Event>> emptye() {
    return std::make_unique<EmptyComparison<Event>>();
}


template<typename T>
std::unique_ptr<FixedSizeComparison<T>> size(std::size_t n) {
    return std::make_unique<FixedSizeComparison<T>>(1, n);
}


inline std::unique_ptr<FixedSizeComparison<Trigger>> sizet(std::size_t n) {
    return std::make_unique<FixedSizeComparison<Trigger>>(1, n);
}


// ==============================================================================================

template<typename T>
std::unique_ptr<VoiceComparison<T>> custom_comparison(const std::function<bool(const Voice<T>&)>& f) {
    return std::make_unique<VoiceComparison<T>>(f);
}


// ==============================================================================================
// TRIGGER COMPARISONS
// ==============================================================================================

inline std::unique_ptr<VoiceComparison<Trigger>> containst(Trigger::Type type
                                                           , std::optional<std::size_t> id = std::nullopt) {
    return std::make_unique<VoiceComparison<Trigger>>([=](const Voice<Trigger>& v) {
        if (id) {
            return Trigger::contains(v, type, *id);
        }
        return Trigger::contains(v, type);
    });
}


inline std::unique_ptr<VoiceComparison<Trigger>> containst_on(std::optional<std::size_t> id = std::nullopt) {
    return containst(Trigger::Type::pulse_on, id);
}


inline std::unique_ptr<VoiceComparison<Trigger>> containst_off(std::optional<std::size_t> id = std::nullopt) {
    return containst(Trigger::Type::pulse_off, id);
}


inline std::unique_ptr<VoiceComparison<Trigger>> equalst(Trigger::Type type
                                                         , std::optional<std::size_t> id = std::nullopt) {
    return std::make_unique<VoiceComparison<Trigger>>([=](const Voice<Trigger>& v) {
        if (v.size() != 1)
            return false;

        if (id) {
            return Trigger::contains(v, type, *id);
        }
        return Trigger::contains(v, type);
    });
}


inline std::unique_ptr<VoiceComparison<Trigger>> equalst_on(std::optional<std::size_t> id = std::nullopt) {
    return equalst(Trigger::Type::pulse_on, id);
}


inline std::unique_ptr<VoiceComparison<Trigger>> equalst_off(std::optional<std::size_t> id = std::nullopt) {
    return equalst(Trigger::Type::pulse_off, id);
}


inline std::unique_ptr<VoiceComparison<Trigger>> sortedt() {
    return std::make_unique<VoiceComparison<Trigger>>([](const Voice<Trigger>& v) {
        return v.is_sorted();
    });
}


// ==============================================================================================
// EVENT COMPARISONS
// ==============================================================================================

inline std::unique_ptr<VoiceComparison<Event>> equals_chord(const Vec<NoteComparator>& cs) {
    return std::make_unique<VoiceComparison<Event>>([=](const Voice<Event>& v) {
        return NoteComparator::matches_chord(cs, v);
    });
}

inline std::unique_ptr<VoiceComparison<Event>> contains_note(const NoteComparator& c) {
    return std::make_unique<VoiceComparison<Event>>([=](const Voice<Event>& v) {
        return NoteComparator::contains(c, v);
    });
}


inline std::unique_ptr<VoiceComparison<Event>> contains_chord(const Vec<NoteComparator>& cs) {
    return std::make_unique<VoiceComparison<Event>>([=](const Voice<Event>& v) {
        return NoteComparator::contains_chord(cs, v);
    });
}
}

#endif //TESTUTILS_C1M_H
