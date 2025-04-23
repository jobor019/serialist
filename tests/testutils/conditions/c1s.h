
#ifndef TESTUTILS_C1S_H
#define TESTUTILS_C1S_H
#include "condition.h"


/**
 * Sequential conditions for Voices<T> of size (S x 1), i.e. voices.size() == S and voices[s].size() == 1 for all s in S
 */

namespace serialist::test::c1s {

template<typename T>
std::unique_ptr<FixedSizeComparison<T>> size(std::size_t n) {
    return std::make_unique<FixedSizeComparison<T>>(n, 1);
}


inline std::unique_ptr<FixedSizeComparison<Trigger>> sizet(std::size_t n) {
    return std::make_unique<FixedSizeComparison<Trigger>>(n, 1);
}


// ==============================================================================================

template<typename T>
std::unique_ptr<VoicesComparison<T>> eq(const Vec<T>& expected) {
    return std::make_unique<VoicesComparison<T>>([=](const Voices<T>& v) {

        // TODO: We should implment a base class for these size conditions
        if (v.size() != expected.size())
            return false;

        for (std::size_t i = 0; i < v.size(); i++)
            if (v[i].size() != 1)
                return false;

        for (std::size_t i = 0; i < v.size(); i++)
            if (v[i][0] != expected[i])
                return false;

        return true;
    });
}



template<typename T>
std::unique_ptr<VoicesComparison<Facet>> eqf(const Vec<Facet>& expected) {
    return eq<Facet>(expected.as_type<Facet>());
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
