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


TEST_CASE("MakeNote: chord", "[make_note]") {
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

TEST_CASE("MakeNote: Overlapping pulses, legato > 1.0 (R1.1.4)", "[makenote]") {
    MakeNoteWrapper w;
    w.velocity.set_values(100);

    auto& nn = w.note_number;
    auto& ch = w.channel;
    auto& trigger = w.trigger;

    NodeRunner runner{&w.make_note_node};


    SECTION("Overlapping pulses on the same note yield note off only when the last pulse is released ") {
        nn.set_values(60);
        ch.set_values(1);

        auto pulse_on1 = Trigger::pulse_on();
        auto pulse_on2 = Trigger::pulse_on();

        trigger.set_values(pulse_on1);

        // NoteOn associated with pulse_on1
        REQUIRE_THAT(runner.step(), m11::eq_note(NC::on(60, 100, 1)));

        // NoteOn associated with pulse_on2
        trigger.set_values(pulse_on2);
        REQUIRE_THAT(runner.step(), m11::eq_note(NC::on(60, 100, 1)));

        // Releasing pulse_on1 shouldn't yield a NoteOff as pulse_on2 is held for the same nn/ch
        trigger.set_values(Trigger::pulse_off(pulse_on1.get_id()));
        REQUIRE_THAT(runner.step(), m1m::emptye());

        // Releasing pulse_on2 should yield a NoteOff as it's the last one associated with given nn/ch
        trigger.set_values(Trigger::pulse_off(pulse_on2.get_id()));
        REQUIRE_THAT(runner.step(), m11::eq_note(NC::off(60, 1)));

    }

    SECTION("Overlapping pulses on different notes are not affected") {
        ch.set_values(1);

        auto pulse_on1 = Trigger::pulse_on();
        auto pulse_on2 = Trigger::pulse_on();

        trigger.set_values(pulse_on1);
        nn.set_values(60);

        // NoteOn(60) associated with pulse_on1
        REQUIRE_THAT(runner.step(), m11::eq_note(NC::on(60, 100, 1)));

        // NoteOn(61) associated with pulse_on2
        trigger.set_values(pulse_on2);
        nn.set_values(61);
        REQUIRE_THAT(runner.step(), m11::eq_note(NC::on(61, 100, 1)));

        // Releasing pulse_on1 should yield a NoteOff as it's not associated with the same nn
        trigger.set_values(Trigger::pulse_off(pulse_on1.get_id()));
        REQUIRE_THAT(runner.step(), m11::eq_note(NC::off(60, 1)));

        // Releasing pulse_on2 should yield a NoteOff
        trigger.set_values(Trigger::pulse_off(pulse_on2.get_id()));
        REQUIRE_THAT(runner.step(), m11::eq_note(NC::off(61, 1)));

    }

    SECTION("Overlapping pulses on different channels are not affected") {
        nn.set_values(60);

        auto pulse_on1 = Trigger::pulse_on();
        auto pulse_on2 = Trigger::pulse_on();

        trigger.set_values(pulse_on1);
        ch.set_values(1);

        // NoteOn(60, _, 1) associated with pulse_on1
        REQUIRE_THAT(runner.step(), m11::eq_note(NC::on(60, 100, 1)));

        // NoteOn(60, _, 2) associated with pulse_on2
        trigger.set_values(pulse_on2);
        ch.set_values(2);
        REQUIRE_THAT(runner.step(), m11::eq_note(NC::on(60, 100, 2)));

        // Releasing pulse_on1 should yield a NoteOff as it's not associated with the same ch
        trigger.set_values(Trigger::pulse_off(pulse_on1.get_id()));
        REQUIRE_THAT(runner.step(), m11::eq_note(NC::off(60, 1)));

        // Releasing pulse_on2 should yield a NoteOff
        trigger.set_values(Trigger::pulse_off(pulse_on2.get_id()));
        REQUIRE_THAT(runner.step(), m11::eq_note(NC::off(60, 2)));
    }
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


TEST_CASE("MakeNote: Trigger Broadcasting (R1.2.5)", "[make_note]") {
    MakeNoteWrapper w;

    NodeRunner runner{&w.make_note_node};
    auto& nn = w.note_number;
    auto& trigger = w.trigger;

    w.velocity.set_values(100);
    w.channel.set_values(1);
    w.num_voices.set_value(0);


    SECTION("Notes from previously broadcasted triggers are flushed when incoming trigger usurp") {
        // Trigger voice count: 2 (both on)
        trigger.set_values(Voices<Trigger>::transposed({Trigger::pulse_on(), Trigger::pulse_on()}));

        // Number of notes: 3
        w.note_number.set_values(Voices<NoteNumber>::transposed({60, 61, 62}));

        // We expect three voices, all on, with notes 60, 61, 62, where the last voice is
        // associated with the first trigger due to broadcasting
        auto r = runner.step();
        REQUIRE_THAT(r, mms::sizee(3));
        REQUIRE_THAT(r, mms::voice_equals(0, NC::on(60)));
        REQUIRE_THAT(r, mms::voice_equals(1, NC::on(61)));
        REQUIRE_THAT(r, mms::voice_equals(2, NC::on(62)));

        // Number of triggers change from 2 to 3
        trigger.set_values(Voices<Trigger>{
            {}
            , {}
            , {Trigger::pulse_on()}
        });

        // Since the trigger count changed, the last voice (which was previously active due to a broadcast trigger)
        // should be flushed when the new trigger usurps the broadcast one, and a new note on should be generated
        r = runner.step();
        REQUIRE_THAT(r, mms::voice_equals(0, NC::empty()));
        REQUIRE_THAT(r, mms::voice_equals(1, NC::empty()));
        REQUIRE_THAT(r, mms::voice_equals(2, {NC::off(62), NC::on(62)}));

        // Note: if this wasn't the case, we'd risk infinitely stuck triggers as the previously broadcasted trigger
        //       would linger forever, as voice 2 will no longer be associated with voice 0 when the pulse_off is sent
        //       on voice 0.
    }

    SECTION("Notes from broadcast triggers are flushed when changes in trigger count lead to re-association") {
        // Trigger voice count: 2 (both on)
        trigger.set_values(Voices<Trigger>::transposed({Trigger::pulse_on(), Trigger::pulse_on()}));

        // Number of notes: 7
        w.note_number.set_values(Voices<NoteNumber>::transposed({60, 61, 62, 63, 64, 65, 66}));

        // We expect seven voices, all on, with notes 60, 61, 62 63, 64, 65, 66.
        // The triggers associated with each voice are (by index) [ 0, 1, 0, 1, 0, 1, 0 ]
        auto r = runner.step();
        REQUIRE_THAT(r, mms::sizee(7));
        REQUIRE_THAT(r, mms::voice_equals(0, NC::on(60)));
        REQUIRE_THAT(r, mms::voice_equals(1, NC::on(61)));
        REQUIRE_THAT(r, mms::voice_equals(2, NC::on(62)));
        REQUIRE_THAT(r, mms::voice_equals(3, NC::on(63)));
        REQUIRE_THAT(r, mms::voice_equals(4, NC::on(64)));
        REQUIRE_THAT(r, mms::voice_equals(5, NC::on(65)));
        REQUIRE_THAT(r, mms::voice_equals(6, NC::on(66)));

        // Number of triggers change from 2 to 3
        trigger.set_values(Voices<Trigger>{
            {}
            , {}
            , {Trigger::pulse_on()}
        });

        // Since the trigger count changed, the way the voices are associated with triggers has changed. We now have
        // [ 0, 1, 2, 0, 1, 2, 0 ]. Which, compared to the intial (written here again for reference):
        // [ 0, 1, 0, 1, 0, 1, 0 ], yields the following voice changes:
        // [ -, -, X, X, X, X, - ]. We therefore expect voices 2, 3, 4 and 5 to be flushed due to broadcast changes, and
        // [ -, -, X, -, -, X, - ]  i.e voices 2 and 5 to generate new pulses due to the new trigger
        r = runner.step();
        REQUIRE_THAT(r, mms::voice_equals(0, NC::empty()));
        REQUIRE_THAT(r, mms::voice_equals(1, NC::empty()));
        REQUIRE_THAT(r, mms::voice_equals(2, {NC::off(62), NC::on(62)}));
        REQUIRE_THAT(r, mms::voice_equals(3, NC::off(63)));
        REQUIRE_THAT(r, mms::voice_equals(4, NC::off(64)));
        REQUIRE_THAT(r, mms::voice_equals(5, {NC::off(65), NC::on(65)}));
        REQUIRE_THAT(r, mms::voice_equals(6, NC::empty()));
    }


}
