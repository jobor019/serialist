
#ifndef TESTUTILS_C1S_H
#define TESTUTILS_C1S_H
#include "condition.h"


namespace serialist::test::c1s {

template<typename T>
std::unique_ptr<FixedSizeComparison<T>> size(std::size_t n) {
    return std::make_unique<FixedSizeComparison<T>>(n, 1);
}


inline std::unique_ptr<FixedSizeComparison<Trigger>> sizet(std::size_t n) {
    return std::make_unique<FixedSizeComparison<Trigger>>(n, 1);
}

// ==============================================================================================
// EVENTS
// ==============================================================================================

inline std::unique_ptr<VoicesComparison<Event>> equals_sequence(const Vec<NoteComparator>& cs) {
    return std::make_unique<VoicesComparison<Event>>([=](const Voices<Event>& vs) {
        return NoteComparator::matches_sequence(cs, vs);
    });
}


}
#endif //TESTUTILS_C1S_H
