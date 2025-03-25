#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "core/policies/policies.h"
#include "core/generatives/make_note.h"

using namespace serialist;

TEST_CASE("MakeNote ctor", "[make_note]") {
    MakeNote mn;
}


TEST_CASE("MakeNote chord", "[make_note]") {
    MakeNote mn;
    auto pulse_on = Trigger::pulse_on();
    auto note_on = mn.process({pulse_on}, {60, 64, 67}, 100, {0});

    REQUIRE(note_on.size() == 3);
    REQUIRE(note_on.all([](const Event& event) { return event.is<MidiNoteEvent>(); }));
    REQUIRE(note_on.contains([](const Event& event) { return event.as<MidiNoteEvent>().note_number == 60; } ));
    REQUIRE(note_on.contains([](const Event& event) { return event.as<MidiNoteEvent>().note_number == 64; } ));
    REQUIRE(note_on.contains([](const Event& event) { return event.as<MidiNoteEvent>().note_number == 67; } ));
    REQUIRE(note_on.all([](const Event& event) { return event.as<MidiNoteEvent>().velocity == 100; }));
    REQUIRE(note_on.all([](const Event& event) { return event.as<MidiNoteEvent>().channel == 0; }));

    auto note_off = mn.process({Trigger::pulse_off(pulse_on.get_id())}, {}, 100, {0});

    REQUIRE(note_off.size() == 3);
    REQUIRE(note_off.all([](const Event& event) { return event.is<MidiNoteEvent>(); }));
    REQUIRE(note_off.contains([](const Event& event) { return event.as<MidiNoteEvent>().note_number == 60; } ));
    REQUIRE(note_off.contains([](const Event& event) { return event.as<MidiNoteEvent>().note_number == 64; } ));
    REQUIRE(note_off.contains([](const Event& event) { return event.as<MidiNoteEvent>().note_number == 67; } ));
    REQUIRE(note_off.all([](const Event& event) { return event.as<MidiNoteEvent>().velocity == 0; }));
    REQUIRE(note_off.all([](const Event& event) { return event.as<MidiNoteEvent>().channel == 0; }));
}

TEST_CASE("MakeNoteWrapper ctor") {
    MakeNoteWrapper mnw;
}