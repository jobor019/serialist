
//#include <iostream>
//#include "core/oscillator.h"
//#include "core/unit_pulse.h"
//#include "core/random_pulsator.h"
//#include "core/sequence.h"
//#include "core/distributor.h"
#include "core/algo/voice/multi_voiced.h"
#include "core/algo/pitch/notes.h"
#include "core/events.h"
#include "core/allocator.h"


int main() {
    Vec<double> v;

    {
        v = Vec<double>::range(0.0, 1.0, 0.1).multiply(8.0);
    }

    v.print();
}