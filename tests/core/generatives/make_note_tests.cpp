#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "core/policies/policies.h"
#include "node_runner.h"
#include "core/generatives/make_note.h"

#include "generators.h"
#include "matchers/m1m.h"
#include "matchers/m11.h"
#include "matchers/m1s.h"
#include "matchers/mms.h"


using namespace serialist;
using namespace serialist::test;

using NC = NoteComparator;


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


template<typename T = uint32_t>
Voices<T> single_note(T v = 60) { return Voices<T>::singular(v); }

template<typename T = uint32_t>
Voices<T> single_chord(T first = 60, std::size_t count = 3) {
    Vec<T> values = Vec<T>::linspace(first, first + count - 1, count);
    return Voices<T>::singular(values);
}

template<typename T = uint32_t>
Voices<T> sequence(T first = 60, std::size_t count = 3) {
    Vec<T> values = Vec<T>::linspace(first, first + count - 1, count);
    return Voices<T>::transposed(values);
}


TEST_CASE("MakeNote: automatic number of voices when num_voices=0 (R1.2.1)", "[make_note]") {
    MakeNoteWrapper w;

    NodeRunner runner{&w.make_note_node};
    auto& nn = w.note_number;
    auto& vel = w.velocity;
    auto& ch = w.channel;
    auto& trigger = w.trigger;
    w.num_voices.set_value(0);

    nn.set_values(single_note());
    vel.set_values(single_note(100u));
    ch.set_values(single_note(1u));
    trigger.set_values(Trigger::pulse_on());

    SECTION("Single note") {
        REQUIRE_THAT(runner.step(), m1m::size<Event>(1));
    }

    SECTION("Chord") {
        SECTION("nn") {
            nn.set_values(single_chord(60u, 5));
            auto r = runner.step();
            REQUIRE_THAT(runner.step(), m1m::size<Event>(5)); // m1m: single voice with size 1
        }
        SECTION("vel") {
            // velocity is ignored for chords: multiple notes with different velocities makes no sense
            vel.set_values(single_chord(100u, 5));
            REQUIRE_THAT(runner.step(), m1m::size<Event>(1));
        }
        SECTION("ch") {
            ch.set_values(single_chord(1u, 5));
            REQUIRE_THAT(runner.step(), m1m::size<Event>(5));
        }
    }

    SECTION("Sequence") {
        SECTION("nn") {
            nn.set_values(sequence(60u, 5));
            REQUIRE_THAT(runner.step(), m1s::size<Event>(5)); // m1s: multiple voices with size 1 each
        }
        SECTION("vel") {
            vel.set_values(sequence(100u, 5));
            REQUIRE_THAT(runner.step(), m1s::size<Event>(5));
        }
        SECTION("ch") {
            ch.set_values(sequence(1u, 5));
            REQUIRE_THAT(runner.step(), m1s::size<Event>(5));
        }
    }
}



TEST_CASE("MakeNote: Flush when number of channels decreases (R1.2.3)", "[make_note]") {
    MakeNoteWrapper w;

    NodeRunner runner{&w.make_note_node};
    auto& nn = w.note_number;
    auto& vel = w.velocity;
    auto& ch = w.channel;
    auto& trigger = w.trigger;
    w.num_voices.set_value(0);

    // All parameters default to 3 voices
    nn.set_values(sequence(60u, 3));
    vel.set_values(sequence(100u, 3));
    ch.set_values(sequence(1u, 3));

    // size of triggers is 3, but since all voices are pulse_on, this should lead to 6 triggers when resized/matched
    auto t = Trigger::pulse_on();
    trigger.set_values(Voices<Trigger>::repeated(t, 3));
    auto trigger_id = t.get_id();

    SECTION("Note number decreases") {
        // First increase size beyond the others
        nn.set_values(sequence(60u, 6));
        auto r = runner.step();
        REQUIRE_THAT(r, m1s::size<Event>(6));
        REQUIRE_THAT(r, m1s::equals_sequence({{60}, {61}, {62}, {63}, {64}, {65}}));

        // Size change but empty trigger vector: no flushing should be triggered
        nn.set_values(sequence(30u, 3));
        trigger.set_values(Voices<Trigger>::zeros(3));
        REQUIRE_THAT(runner.step(), m1m::emptye());

        // Trigger only in voice 0 (non-empty trigger vector): expected output:
        //   - Voice 0:    NoteOff(60), NoteOn(30)
        //   - Voices 1-2: No output (since there are no triggers in these voices)
        //   - Voices 3-6: Flushed NoteOffs (63, 64, 65) due to resize
        trigger.set_values(Voices<Trigger>{
            {Trigger::pulse_off(trigger_id), Trigger::pulse_on()}
            , {}
            , {}
        });

        r = runner.step();
        REQUIRE_THAT(r, mms::sizee(6));
        REQUIRE_THAT(r, mms::voice_equals(0, {NC::off(60), NC::on(30)}));
        REQUIRE_THAT(r, mms::voice_equals(1, NC::empty()));
        REQUIRE_THAT(r, mms::voice_equals(2, NC::empty()));
        REQUIRE_THAT(r, mms::voice_equals(3, NC::off(63)));
        REQUIRE_THAT(r, mms::voice_equals(4, NC::off(64)));
        REQUIRE_THAT(r, mms::voice_equals(5, NC::off(65)));


        // next step: new size should be 3, output as expected under normal operation (no resize changes)
        trigger.set_values(Voices<Trigger>{
            {}
            , {Trigger::pulse_off(trigger_id)}
            , {}
        });
        r = runner.step();
        REQUIRE_THAT(r, mms::sizee(3));
        REQUIRE_THAT(r, m1s::equals_sequence({{}, NC::off(61), {}}));
    }


    SECTION("Trigger count decreases") {
        // First increase size beyond the others
        trigger.set_values(Voices<Trigger>::repeated(t, 6));
        auto r = runner.step();
        REQUIRE_THAT(r, m1s::size<Event>(6));
        REQUIRE_THAT(r, m1s::equals_sequence({{60}, {61}, {62}, {60}, {61}, {62}}));

        // Size change but empty trigger vector: no flushing should be triggered
        trigger.set_values(Voices<Trigger>::zeros(3));
        REQUIRE_THAT(runner.step(), m1m::emptye());

        // Trigger only in voice 0 (non-empty trigger vector): expected output:
        //   - Voice 0:    NoteOff(60), NoteOn(60)
        //   - Voices 1-2: No output (since there are no triggers in these voices)
        //   - Voices 3-6: Flushed NoteOffs (60, 60, 60) due to resize
        trigger.set_values(Voices<Trigger>{
            {Trigger::pulse_off(trigger_id), Trigger::pulse_on()}
            , {}
            , {}
        });

        r = runner.step();
        REQUIRE_THAT(r, mms::sizee(6));
        REQUIRE_THAT(r, mms::voice_equals(0, {NC::off(60), NC::on(60)}));
        REQUIRE_THAT(r, mms::voice_equals(1, NC::empty()));
        REQUIRE_THAT(r, mms::voice_equals(2, NC::empty()));
        REQUIRE_THAT(r, mms::voice_equals(3, NC::off(60)));
        REQUIRE_THAT(r, mms::voice_equals(4, NC::off(61)));
        REQUIRE_THAT(r, mms::voice_equals(5, NC::off(62)));


        // next step: new size should be 3, output as expected under normal operation (no resize changes)
        trigger.set_values(Voices<Trigger>{
            {}
            , {Trigger::pulse_off(trigger_id)}
            , {}
        });
        r = runner.step();
        REQUIRE_THAT(r, mms::sizee(3));
        REQUIRE_THAT(r, m1s::equals_sequence({{}, NC::off(61), {}}));
    }
}
