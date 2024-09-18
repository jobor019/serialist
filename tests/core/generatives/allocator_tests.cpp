//#include <catch2/catch_test_macros.hpp>
//#include <catch2/matchers/catch_matchers_floating_point.hpp>
//#include "core/generatives/allocator.h"
//
//TEST_CASE("Allocator - Basic Functionality") {
//    Allocator allocator(0);
//    auto pc_range = PitchClassRange({0, 4, 7}, 12);
//    allocator.set_pitch_classes(pc_range);
//    allocator.set_spectrum_distribution(Vec<double>{0.2, 0.8});
//
//    std::size_t num_voices = 1;
//    REQUIRE(allocator.resize(num_voices).is_empty_like());
//
//    SECTION("Test voice allocation and binding") {
//        for (int i = 0; i < 1000; ++i) {
//            auto triggers = Voices<Trigger>::singular(Trigger::pulse_on);
//            Voices<NoteNumber> note_ons = allocator.bind(triggers, num_voices);
//            REQUIRE(note_ons.size() == 1);
//            REQUIRE(note_ons.first().has_value());
//            REQUIRE(*note_ons.first() >= pitch::MIN_NOTE);
//            REQUIRE(*note_ons.first() <= pitch::MAX_NOTE);
//            REQUIRE(pc_range.is_in(*note_ons.first()));
//
//            triggers = Voices<Trigger>::singular(Trigger::pulse_off);
//            Voices<NoteNumber> note_offs = allocator.release(triggers, num_voices);
//            REQUIRE(note_offs == note_ons);
//        }
//    }
//
//    SECTION("Test release on new trigger") {
//        auto triggers = Voices<Trigger>::singular(Trigger::pulse_on);
//        auto previous_note_ons_on = allocator.bind(triggers, num_voices);
//        for (int i = 0; i < 1000; ++i) {
//
//            // test that it releases even if no explicit pulse_off is receivec
//            auto note_offs = allocator.release(triggers, num_voices);
//            REQUIRE(note_offs.size() == 1);
//            REQUIRE(note_offs.first().has_value());
//            REQUIRE(*note_offs.first() >= pitch::MIN_NOTE);
//            REQUIRE(*note_offs.first() <= pitch::MAX_NOTE);
//            REQUIRE(pc_range.is_in(*note_offs.first()));
//            REQUIRE(note_offs == previous_note_ons_on);
//
//
//            previous_note_ons_on = allocator.bind(triggers, num_voices);
//        }
//    }
//
//}
//
//
//TEST_CASE("Allocator - Multiple Voices") {
//    Allocator allocator(0);
//    auto pc_range = PitchClassRange({0, 4, 7}, 12);
//    allocator.set_pitch_classes(pc_range);
//    allocator.set_spectrum_distribution(Vec<double>{0.2, 0.8});
//
//    SECTION("Multiple voices - Homophony") {
//        std::size_t num_voices = 18;
//        REQUIRE(allocator.resize(num_voices).is_empty_like());
//
//        for (int i = 0; i < 1000; ++i) {
//            auto triggers = Voices<Trigger>::repeated(Trigger::pulse_on, num_voices);
//            Voices<NoteNumber> note_ons = allocator.bind(triggers, num_voices);
//            REQUIRE(note_ons.size() == num_voices);
//            REQUIRE(note_ons.firsts().all([](const auto& note) { return note.has_value(); }));
//            REQUIRE(note_ons.firsts().all([](const auto& note) { return *note >= pitch::MIN_NOTE; }));
//            REQUIRE(note_ons.firsts().all([](const auto& note) { return *note <= pitch::MAX_NOTE; }));
//            REQUIRE(note_ons.firsts().all([&pc_range](const auto& note) { return pc_range.is_in(*note); }));
//
//            triggers = Voices<Trigger>::repeated(Trigger::pulse_off, num_voices);
//            Voices<NoteNumber> note_offs = allocator.release(triggers, num_voices);
//            REQUIRE(note_offs == note_ons);
//        }
//    }
//
//    SECTION("Multiple voices - Polyphony") {
//        std::size_t num_voices = 3;
//        REQUIRE(allocator.resize(num_voices).is_empty_like());
//
//        // Note ons
//        auto triggers = Voices<Trigger>::singular(Trigger::pulse_on, num_voices);
//        Voices<NoteNumber> note_ons = allocator.bind(triggers, num_voices);
//        REQUIRE(note_ons.size() == num_voices);
//        REQUIRE(note_ons.first().has_value());
//        REQUIRE_FALSE(note_ons.firsts()[1].has_value());
//        REQUIRE_FALSE(note_ons.firsts()[2].has_value());
//
//        triggers = Voices<Trigger>::one_hot(Trigger::pulse_on, 1, num_voices);
//        note_ons = allocator.bind(triggers, num_voices);
//        REQUIRE(note_ons.size() == num_voices);
//        REQUIRE_FALSE(note_ons.first().has_value());
//        REQUIRE(note_ons.firsts()[1].has_value());
//        REQUIRE_FALSE(note_ons.firsts()[2].has_value());
//
//        triggers = Voices<Trigger>::one_hot(Trigger::pulse_on, 2, num_voices);
//        note_ons = allocator.bind(triggers, num_voices);
//        REQUIRE(note_ons.size() == num_voices);
//        REQUIRE_FALSE(note_ons.first().has_value());
//        REQUIRE_FALSE(note_ons.firsts()[1].has_value());
//        REQUIRE(note_ons.firsts()[2].has_value());
//
//        // Note offs
//        triggers = Voices<Trigger>::one_hot(Trigger::pulse_off, 1, num_voices);
//        auto note_offs = allocator.release(triggers, num_voices);
//        REQUIRE(note_offs.size() == num_voices);
//        REQUIRE_FALSE(note_offs.first().has_value());
//        REQUIRE(note_offs.firsts()[1].has_value());
//        REQUIRE_FALSE(note_offs.firsts()[2].has_value());
//
//        triggers = Voices<Trigger>::one_hot(Trigger::pulse_off, 0, num_voices);
//        note_offs = allocator.release(triggers, num_voices);
//        REQUIRE(note_offs.size() == num_voices);
//        REQUIRE(note_offs.first().has_value());
//        REQUIRE_FALSE(note_offs.firsts()[1].has_value());
//        REQUIRE_FALSE(note_offs.firsts()[2].has_value());
//
//        triggers = Voices<Trigger>::one_hot(Trigger::pulse_off, 2, num_voices);
//        note_offs = allocator.release(triggers, num_voices);
//        REQUIRE(note_offs.size() == num_voices);
//        REQUIRE_FALSE(note_offs.first().has_value());
//        REQUIRE_FALSE(note_offs.firsts()[1].has_value());
//        REQUIRE(note_offs.firsts()[2].has_value());
//
//
//    }
//
//    SECTION("Test spectrum distribution and pitch classes") {
//        std::size_t num_bands = 4;
//        allocator.set_spectrum_distribution(Vec<double>::one_hot(1.0, 0, num_bands));
//        pc_range = PitchClassRange({0, 1, 2}, 12);
//        allocator.set_pitch_classes(pc_range);
//
//        auto low_thresh = allocator.get_classifier().start_of(0);
//        auto high_thresh = allocator.get_classifier().end_of(0);
//        REQUIRE(low_thresh == pitch::MIN_NOTE);
//        REQUIRE(std::abs(static_cast<int>(high_thresh - (pitch::MIN_NOTE + (pitch::NOTE_RANGE / num_bands)))) >= 1);
//
//        std::size_t num_voices = 1;
//        REQUIRE(allocator.resize(num_voices).is_empty_like());
//
//        auto triggers = Voices<Trigger>::singular(Trigger::pulse_on);
//        auto previous_note_ons_on = allocator.bind(triggers, num_voices);
//        for (int i = 0; i < 10000; ++i) {
//            auto note_offs = allocator.release(triggers, num_voices);
//            REQUIRE(note_offs.size() == 1);
//            REQUIRE(note_offs.first().has_value());
//            REQUIRE(*note_offs.first() >= low_thresh);
//            REQUIRE(*note_offs.first() < high_thresh);
//            REQUIRE(pc_range.is_in(*note_offs.first()));
//            REQUIRE(note_offs == previous_note_ons_on);
//
//            previous_note_ons_on = allocator.bind(triggers, num_voices);
//        }
//
//
//    }SECTION("Test empty intersection of pitch classes and distribution") {
//        std::size_t num_voices = 1;
//        REQUIRE(allocator.resize(num_voices).is_empty_like());
//
//        std::size_t num_bands = pitch::NOTE_RANGE;
//        allocator.set_spectrum_distribution(Vec<double>::one_hot(1.0, 0, num_bands));
//
//        auto low_thresh = allocator.get_classifier().start_of(0);
//        auto high_thresh = allocator.get_classifier().end_of(0);
//        REQUIRE(low_thresh == pitch::MIN_NOTE);
//        REQUIRE(high_thresh == pitch::MIN_NOTE + 1);
//
//
//        auto only_valid_pc = pc_range.classify(low_thresh);
//        auto pcs = Vec<NoteNumber>::range(12).remove(only_valid_pc);
//        allocator.set_pitch_classes(PitchClassRange(pcs, 12, 0));
//
//        auto triggers = Voices<Trigger>::singular(Trigger::pulse_on);
//        REQUIRE(allocator.bind(triggers, num_voices).is_empty_like());
//    }
//}
//
//
//TEST_CASE("Distributions") {
//    Allocator allocator(0);
//    auto pc_range = PitchClassRange(Vec<NoteNumber>::range(12), 12);
//    allocator.set_pitch_classes(pc_range);
//
//    SECTION("Binary distribution") {
//        auto first_band_weight = 0.2;
//
//        allocator.set_spectrum_distribution(Vec<double>{first_band_weight, 1 - first_band_weight});
//        auto thresh = allocator.get_classifier().end_of(0);
//
//        std::size_t num_voices = 10;
//        REQUIRE(allocator.resize(num_voices).is_empty_like());
//
//
//        std::size_t n_iterations = 1000;
//
//        auto balance_per_chord = Vec<double>::allocated(n_iterations);
//
//        auto triggers = Voices<Trigger>::repeated(Trigger::pulse_on, num_voices);
//        for (std::size_t i = 0; i < n_iterations; ++i) {
//            auto note_ons = allocator.bind(triggers, num_voices);
//            auto count = note_ons.flattened().count([&thresh](const auto& note) { return note < thresh; });
//            balance_per_chord.append(static_cast<double>(count) / static_cast<double>(num_voices));
//            auto note_offs = allocator.release(triggers, num_voices);
//        }
//
//        for (const auto& balance: balance_per_chord) {
//            REQUIRE_THAT(balance, Catch::Matchers::WithinRel(first_band_weight, 0.1));
//        }
//    }
//
//    SECTION("Uneven distribution") {
//        auto first_band_weight = 0.9;
//        std::size_t n_bands = 10;
//
//        auto weights = Vec<double>::ones(n_bands).multiply((1.0 - first_band_weight) / static_cast<double>(n_bands));
//        weights[0] = first_band_weight;
//        allocator.set_spectrum_distribution(weights);
//        auto classifier = allocator.get_classifier();
//        auto num_classes = classifier.get_num_classes();
//
//        std::size_t num_voices = 100;
//        REQUIRE(allocator.resize(num_voices).is_empty_like());
//
//        std::size_t num_iterations = 100;
//
//        auto class_occurrences = Vec<std::size_t>::zeros(num_classes);
//
//        auto triggers = Voices<Trigger>::repeated(Trigger::pulse_on, num_voices);
//
//        for (std::size_t i = 0; i < num_iterations; ++i) {
//            auto note_ons = allocator.bind(triggers, num_voices);
//            auto class_count = Histogram<std::size_t>::with_discrete_bins(
//                    classifier.classify(note_ons.flattened())
//                    , Vec<std::size_t>::range(num_classes)).get_counts();
//            class_occurrences += class_count;
//            REQUIRE_THAT(static_cast<double>(class_count[0]) / static_cast<double>(num_voices)
//                         , Catch::Matchers::WithinAbs(first_band_weight, 0.1));
//            auto note_offs = allocator.release(triggers, num_voices);
//        }
//
//        auto outcome = class_occurrences.as_type<double>().multiply(
//                1.0 / static_cast<double>(num_iterations * num_voices));
//
//        for (std::size_t i = 0; i < num_classes; ++i) {
//            REQUIRE_THAT(outcome[i], Catch::Matchers::WithinAbs(weights[i], 0.01));
//        }
//
//    }
//}
//
//
//TEST_CASE("Performance") {
//    Allocator allocator(0);
//
//    Vec<std::size_t> voice_counts{1, 10, 100};
//    std::size_t num_iterations = 1000;
//
//    std::cout << "Performance: Time consumption for a single bind/release cycle (num cycles: "
//              << num_iterations << ")\n\n";
//
//    for (const auto& num_voices: voice_counts) {
//        REQUIRE(allocator.resize(num_voices).is_empty_like());
//
//
//        auto triggers = Voices<Trigger>::repeated(Trigger::pulse_on, num_voices);
//
//        auto times = Vec<long long>::allocated(num_iterations);
//        for (std::size_t i = 0; i < num_iterations; ++i) {
//            auto t1 = std::chrono::high_resolution_clock::now();
//            auto note_ons = allocator.bind(triggers, num_voices);
//            auto note_offs = allocator.release(triggers, num_voices);
//            auto t2 = std::chrono::high_resolution_clock::now();
//            times.append(std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
//        }
//
//
//        std::cout << "num_voices: " << num_voices << "\n";
//        std::cout << "    mean: " << times.mean() << " usec\n";
//        std::cout << "    max: " << times.max() << " usec\n";
//        std::cout << "    min: " << times.min() << " usec\n\n";
//    }
//
//
//}