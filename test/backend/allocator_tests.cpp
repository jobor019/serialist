#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/distributor.h"
#include "core/events.h"

TEST_CASE("Allocator - Basic Functionality") {
    Allocator allocator(0);
    auto pc_range = PitchClassRange({0, 4, 7}, 12);
    allocator.set_pitch_classes(pc_range);
    allocator.set_spectrum_distribution(Vec<double>{0.2, 0.8});

    std::size_t num_voices = 1;
    REQUIRE(allocator.resize(num_voices).is_empty_like());

    SECTION("Test voice allocation and binding") {
        for (int i = 0; i < 1000; ++i) {
            auto triggers = Voices<Trigger>::singular(Trigger(0.0, Trigger::Type::pulse_on, 1));
            Voices<NoteNumber> note_ons = allocator.bind(triggers, num_voices);
            REQUIRE(note_ons.size() == 1);
            REQUIRE(note_ons.first().has_value());
            REQUIRE(*note_ons.first() >= pitch::MIN_NOTE);
            REQUIRE(*note_ons.first() <= pitch::MAX_NOTE);
            REQUIRE(pc_range.is_in(*note_ons.first()));

            triggers = Voices<Trigger>::singular(Trigger(0.0, Trigger::Type::pulse_off, 1));
            Voices<NoteNumber> note_offs = allocator.release(triggers, num_voices);
            REQUIRE(note_offs == note_ons);
        }
    }

    SECTION("Test release on new trigger") {
        auto triggers = Voices<Trigger>::singular(Trigger(0.0, Trigger::Type::pulse_on, 1));
        auto previous_note_ons_on = allocator.bind(triggers, num_voices);
        for (int i = 0; i < 1000; ++i) {

            // test that it releases even if no explicit pulse_off is receivec
            auto note_offs = allocator.release(triggers, num_voices);
            REQUIRE(note_offs.size() == 1);
            REQUIRE(note_offs.first().has_value());
            REQUIRE(*note_offs.first() >= pitch::MIN_NOTE);
            REQUIRE(*note_offs.first() <= pitch::MAX_NOTE);
            REQUIRE(pc_range.is_in(*note_offs.first()));
            REQUIRE(note_offs == previous_note_ons_on);


            previous_note_ons_on = allocator.bind(triggers, num_voices);
        }
    }

}

TEST_CASE("Allocator - Multiple Voices") {
    Allocator allocator(0);
    auto pc_range = PitchClassRange({0, 4, 7}, 12);
    allocator.set_pitch_classes(pc_range);
    allocator.set_spectrum_distribution(Vec<double>{0.2, 0.8});

    SECTION("Multiple voices - Homophony") {
        std::size_t num_voices = 18;
        REQUIRE(allocator.resize(num_voices).is_empty_like());

        for (int i = 0; i < 1000; ++i) {
            auto triggers = Voices<Trigger>::repeated(Trigger(0.0, Trigger::Type::pulse_on, 1), num_voices);
            Voices<NoteNumber> note_ons = allocator.bind(triggers, num_voices);
            REQUIRE(note_ons.size() == num_voices);
            REQUIRE(note_ons.fronts().all([](const auto& note){return note.has_value();}));
            REQUIRE(note_ons.fronts().all([](const auto& note){return *note >= pitch::MIN_NOTE;}));
            REQUIRE(note_ons.fronts().all([](const auto& note){return *note <= pitch::MAX_NOTE;}));
            REQUIRE(note_ons.fronts().all([&pc_range](const auto& note){return pc_range.is_in(*note);}));

            triggers = Voices<Trigger>::repeated(Trigger(0.0, Trigger::Type::pulse_off, 1), num_voices);
            Voices<NoteNumber> note_offs = allocator.release(triggers, num_voices);
            REQUIRE(note_offs == note_ons);
        }
    }

    SECTION("Multiple voices - Polyphony") {
        std::size_t num_voices = 3;
        REQUIRE(allocator.resize(num_voices).is_empty_like());

        // Note ons
        auto triggers = Voices<Trigger>::singular(Trigger(0.0, Trigger::Type::pulse_on, 1), num_voices);
        Voices<NoteNumber> note_ons = allocator.bind(triggers, num_voices);
        REQUIRE(note_ons.size() == num_voices);
        REQUIRE(note_ons.first().has_value());
        REQUIRE_FALSE(note_ons.fronts()[1].has_value());
        REQUIRE_FALSE(note_ons.fronts()[2].has_value());

        triggers = Voices<Trigger>::one_hot(Trigger(0.0, Trigger::Type::pulse_on, 1), 1, num_voices);
        note_ons = allocator.bind(triggers, num_voices);
        REQUIRE(note_ons.size() == num_voices);
        REQUIRE_FALSE(note_ons.first().has_value());
        REQUIRE(note_ons.fronts()[1].has_value());
        REQUIRE_FALSE(note_ons.fronts()[2].has_value());

        triggers = Voices<Trigger>::one_hot(Trigger(0.0, Trigger::Type::pulse_on, 1), 2, num_voices);
        note_ons = allocator.bind(triggers, num_voices);
        REQUIRE(note_ons.size() == num_voices);
        REQUIRE_FALSE(note_ons.first().has_value());
        REQUIRE_FALSE(note_ons.fronts()[1].has_value());
        REQUIRE(note_ons.fronts()[2].has_value());

        // Note offs
        triggers = Voices<Trigger>::one_hot(Trigger(0.0, Trigger::Type::pulse_off, 1), 1, num_voices);
        auto note_offs = allocator.release(triggers, num_voices);
        REQUIRE(note_offs.size() == num_voices);
        REQUIRE_FALSE(note_offs.first().has_value());
        REQUIRE(note_offs.fronts()[1].has_value());
        REQUIRE_FALSE(note_offs.fronts()[2].has_value());

        triggers = Voices<Trigger>::one_hot(Trigger(0.0, Trigger::Type::pulse_off, 1), 0, num_voices);
        note_offs = allocator.release(triggers, num_voices);
        REQUIRE(note_offs.size() == num_voices);
        REQUIRE(note_offs.first().has_value());
        REQUIRE_FALSE(note_offs.fronts()[1].has_value());
        REQUIRE_FALSE(note_offs.fronts()[2].has_value());

        triggers = Voices<Trigger>::one_hot(Trigger(0.0, Trigger::Type::pulse_off, 1), 2, num_voices);
        note_offs = allocator.release(triggers, num_voices);
        REQUIRE(note_offs.size() == num_voices);
        REQUIRE_FALSE(note_offs.first().has_value());
        REQUIRE_FALSE(note_offs.fronts()[1].has_value());
        REQUIRE(note_offs.fronts()[2].has_value());


    }

    SECTION("Test spectrum distribution and pitch classes") {
        std::size_t num_bands = 4;
        allocator.set_spectrum_distribution(Vec<double>::one_hot(1.0, 0, num_bands));
        pc_range = PitchClassRange({0, 1, 2}, 12);
        allocator.set_pitch_classes(pc_range);

        auto low_thresh = allocator.get_classifier().start_of(0);
        auto high_thresh = allocator.get_classifier().end_of(0);
        REQUIRE(low_thresh == pitch::MIN_NOTE);
        REQUIRE(std::abs(static_cast<int>(high_thresh -  (pitch::MIN_NOTE + (pitch::NOTE_RANGE/num_bands))))>= 1);

        std::size_t num_voices = 1;
        REQUIRE(allocator.resize(num_voices).is_empty_like());

        auto triggers = Voices<Trigger>::singular(Trigger(0.0, Trigger::Type::pulse_on, 1));
        auto previous_note_ons_on = allocator.bind(triggers, num_voices);
        for (int i = 0; i  < 10000; ++i) {
                auto note_offs = allocator.release(triggers, num_voices);
                REQUIRE(note_offs.size() == 1);
                REQUIRE(note_offs.first().has_value());
                REQUIRE(*note_offs.first() >= low_thresh);
                REQUIRE(*note_offs.first() < high_thresh);
                REQUIRE(pc_range.is_in(*note_offs.first()));
                REQUIRE(note_offs == previous_note_ons_on);

                previous_note_ons_on = allocator.bind(triggers, num_voices);
        }


    }
    SECTION("Test empty intersection of pitch classes and distribution") {
        std::size_t num_voices = 1;
        REQUIRE(allocator.resize(num_voices).is_empty_like());

        std::size_t num_bands = pitch::NOTE_RANGE;
        allocator.set_spectrum_distribution(Vec<double>::one_hot(1.0, 0, num_bands));

        auto low_thresh = allocator.get_classifier().start_of(0);
        auto high_thresh = allocator.get_classifier().end_of(0);
        REQUIRE(low_thresh == pitch::MIN_NOTE);
        REQUIRE(high_thresh == pitch::MIN_NOTE + 1);


        auto only_valid_pc = pc_range.classify(low_thresh);
        auto pcs = Vec<NoteNumber>::range(12).remove(only_valid_pc);
        allocator.set_pitch_classes(PitchClassRange(pcs, 12, 0));

        auto triggers = Voices<Trigger>::singular(Trigger(0.0, Trigger::Type::pulse_on, 1));
        REQUIRE(allocator.bind(triggers, num_voices).is_empty_like());
    }
}

TEST_CASE("Distributions") {
    // TODO
}




