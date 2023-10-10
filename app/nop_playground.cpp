
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
    Allocator allocator(0);
    auto pc_range = PitchClassRange({0, 4, 7}, 12);
    allocator.set_pitch_classes(pc_range);
    allocator.set_spectrum_distribution(Vec<double>{0.2, 0.8});

        std::size_t num_voices = 18;
        allocator.resize(num_voices);

//        for (int i = 0; i < 1000; ++i) {
            auto triggers = Voices<Trigger>::repeated(Trigger(0.0, Trigger::Type::pulse_on, 1), num_voices);
            Voices<NoteNumber> note_ons = allocator.bind(triggers, num_voices);
            std::cout << (note_ons.size() == num_voices) << std::endl;
            std::cout << (note_ons.fronts().all([](const auto& note){return note.has_value();})) << std::endl;
            std::cout << (note_ons.fronts().all([](const auto& note){return *note >= pitch::MIN_NOTE;})) << std::endl;
            std::cout << (note_ons.fronts().all([](const auto& note){return *note <= pitch::MAX_NOTE;})) << std::endl;
            std::cout << (note_ons.fronts().all([&pc_range](const auto& note){return pc_range.is_in(*note);})) << std::endl;

            triggers = Voices<Trigger>::repeated(Trigger(0.0, Trigger::Type::pulse_off, 1), num_voices);
            Voices<NoteNumber> note_offs = allocator.release(triggers, num_voices);
            std::cout << (note_offs == note_ons) << std::endl;
//    }


//    auto seed = 995;
//    Allocator allocator(seed);
//
//
//    auto pitch_classes = Vec<NoteNumber>{0, 4, 7}; // pitch classes should range from 0 to 11 (assuming we use a pivot of 12)
//    auto pivot = 12;
//    allocator.set_pitch_classes(PitchClassRange(pitch_classes, pivot));
//
//    // create weights 6 bands over the scope of pitches from 21 to 108
//    auto weights = Vec<double>{0.5, 0.2, 0.1, 0.1, 0.2, 0.5}.normalize();
//    allocator.set_spectrum_distribution(weights);
//
//    // if number of voices change, we should start by flushing any lingering voices
//    std::size_t num_voices = 4;
//    auto e = allocator.resize(num_voices);
//    assert(e.is_empty_like()); // it should be empty here since we haven't changed the number of voices
//
//
//    // A cycle typically consists of
//    //  (1) updating values (distribution, num_voices, etc.)
//    //  (2) flushing any lingering voices if the number of voices changes
//    //  (3) releasing any voices which receive a pulse_off
//    //  (4) binding any voices which receive a pulse_on
//    for (int i = 0; i < 10; ++i) {
//        // the Trigger constructor takes three arguments. Here, only the second argument is relevant.
//        // It can be either Trigger::Type::pulse_on or Trigger::Type::pulse_off
//        Voices<Trigger> triggers{{{Trigger(0.0, Trigger::Type::pulse_on, 1)}, {}, {}, {}}};
//
//
//        auto note_offs = allocator.release(triggers, num_voices);
//        auto note_ons = allocator.bind(triggers, num_voices);
//
//
//    }
}