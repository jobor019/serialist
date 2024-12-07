
#ifndef TESTUTILS_GENERATORS_H
#define TESTUTILS_GENERATORS_H

#include <algo/facet.h>

#include "algo/random.h"
#include "algo/temporal/time_point.h"


namespace serialist::test {

static inline Random RNG{0}; // Global generator with fixed seed for test consistency


inline TimePoint random_tp(double start_tick, double end_tick) {
    assert(start_tick < end_tick);
    auto t = RNG.next(start_tick, end_tick);
    return TimePoint{t};
}




}

#endif //TESTUTILS_GENERATORS_H
