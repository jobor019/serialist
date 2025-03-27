
#ifndef TESTUTILS_CMS_H
#define TESTUTILS_CMS_H

#include "condition.h"

namespace serialist::test::cms {

template<typename T>
std::unique_ptr<FixedSizeComparison<T>> size(std::size_t voices_size) {
    return std::make_unique<FixedSizeComparison<T>>(voices_size, std::nullopt);
}

inline std::unique_ptr<FixedSizeComparison<Event>> sizee(std::size_t n) { return size<Event>(n); }



// ==============================================================================================
// EVENTS
// ==============================================================================================

inline std::unique_ptr<VoicesComparison<Event>> voice_equals(std::size_t voice_index, const NoteComparator& c) {
    return std::make_unique<VoicesComparison<Event>>([=](const Voices<Event>& vs) {
        return voice_index < vs.size() && NoteComparator::matches_chord({c}, vs[voice_index]);
    });
}


inline std::unique_ptr<VoicesComparison<Event>> voice_equals(std::size_t voice_index, const Vec<NoteComparator>& cs) {
    return std::make_unique<VoicesComparison<Event>>([=](const Voices<Event>& vs) {
        return voice_index < vs.size() && NoteComparator::matches_chord(cs, vs[voice_index]);
    });
}

inline std::unique_ptr<VoicesComparison<Event>> voice_contains(std::size_t voice_index, const NoteComparator& c) {
    return std::make_unique<VoicesComparison<Event>>([=](const Voices<Event>& vs) {
        return voice_index < vs.size() && NoteComparator::contains(c, vs[voice_index]);
    });
}


inline std::unique_ptr<VoicesComparison<Event>> voice_contains(std::size_t voice_index, const Vec<NoteComparator>& cs) {
    return std::make_unique<VoicesComparison<Event>>([=](const Voices<Event>& vs) {
        return voice_index < vs.size() && NoteComparator::contains_chord(cs, vs[voice_index]);
    });
}


}

#endif //TESTUTILS_CMS_H
