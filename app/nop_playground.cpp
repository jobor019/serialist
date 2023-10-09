
//#include <iostream>
//#include "core/oscillator.h"
//#include "core/unit_pulse.h"
//#include "core/random_pulsator.h"
//#include "core/sequence.h"
//#include "core/distributor.h"
#include "core/algo/voice/multi_voiced.h"
#include "core/algo/pitch/notes.h"
#include "core/events.h"
#include "core/distributor.h"


int main() {

//    for (unsigned long i = 0; i < 1000; ++i) {
//        std::cout << "seed: " << i << std::endl;
        Allocator allocator(995);

        std::size_t num_voices = 4;

        auto e = allocator.resize(num_voices);
        assert(e.is_empty_like());


        Voices<Trigger> triggers{{{Trigger(0.0, Trigger::Type::pulse_on, 1)}, {}, {}, {}}};

        auto output = allocator.bind(triggers, num_voices);
        output.print();
//    }


//    Voices<Trigger> triggers2{{{Trigger(0.0, Trigger::Type::pulse_off, 1)}, {}, {}, {}}};
//    auto output2 = allocator.release(triggers2, num_voices);
//    output2.print();

}