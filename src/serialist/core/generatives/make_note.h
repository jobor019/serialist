
#ifndef SERIALISTLOOPER_MAKE_NOTE_H
#define SERIALISTLOOPER_MAKE_NOTE_H

#include "core/event.h"
#include "core/generative.h"
#include "core/algo/pitch/notes.h"
#include "core/temporal/trigger.h"
#include "core/algo/facet.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "sequence.h"
#include "variable.h"

namespace serialist {

class MakeNote {
public:
    Voice<Event> process(const Voice<Trigger>& triggers
                         , const Voice<NoteNumber>& chord
                         , const unsigned int velocity
                         , const unsigned int channel) {
        Voice<Event> events;

        if (auto index = triggers.index([](const Trigger& trigger) {
            return trigger.is(Trigger::Type::pulse_off);
        })) {
            events.extend(process_pulse_off(triggers[*index].get_id()));
        }

        if (auto index = triggers.index([](const Trigger& trigger) {
            return trigger.is(Trigger::Type::pulse_on);
        })) {
            events.extend(process_pulse_on(triggers[*index].get_id(), chord, velocity, channel));
        }

        return events;
    }

    Voice<Event> flush() {
        return m_held_notes.flush()
                .as_type<Event>([](const IdentifiedChanneledHeld& note) {
                    return Event(MidiNoteEvent{note.note, 0, note.channel});
                });
    }

private:

    Voice<Event> process_pulse_on(const std::size_t id
                                  , const Voice<NoteNumber>& notes
                                  , const unsigned int velocity
                                  , const unsigned int channel) {
        auto events = Voice<Event>::allocated(notes.size());
        for (const auto& note: notes) {
            m_held_notes.bind({id, note, channel});
            events.append(Event(MidiNoteEvent{note, velocity, channel}));
        }

        return events;
    }

    Voice<Event> process_pulse_off(std::size_t id) {
        return m_held_notes.get_held_mut()
                .filter_drain([&id](const IdentifiedChanneledHeld& v) {
                    return v.id != id;
                })
                .as_type<Event>([](const IdentifiedChanneledHeld& note) {
                    return Event(MidiNoteEvent{note.note, 0, note.channel});
                });

    }

    HeldNotesWithIds m_held_notes;
};


// ==============================================================================================

class MakeNoteNode : public NodeBase<Event> {
public:
    struct Keys {
        static const inline std::string NOTE_NUMBER = "note_number";
        static const inline std::string VELOCITY = "velocity";
        static const inline std::string CHANNEL = "channel";

        static const inline std::string CLASS_NAME = "makenote";
    };

    MakeNoteNode(const std::string& identifier
                 , ParameterHandler& parent
                 , Node<Trigger>* trigger = nullptr
                 , Node<Facet>* note_number = nullptr
                 , Node<Facet>* velocity = nullptr
                 , Node<Facet>* channel = nullptr
                 , Node<Facet>* enabled = nullptr
                 , Node<Facet>* num_voices = nullptr)
            : NodeBase(identifier, parent, enabled, num_voices, Keys::CLASS_NAME)
              , m_trigger(add_socket(param::properties::trigger, trigger))
              , m_note_number(add_socket(Keys::NOTE_NUMBER, note_number))
              , m_velocity(add_socket(Keys::VELOCITY, velocity))
              , m_channel(add_socket(Keys::CHANNEL, channel)) {}

    Voices<Event> process() override {
        auto t = pop_time();
        if (!t) {
            return m_current_value;
        }

        if (!is_enabled() || !m_trigger.is_connected() || !m_note_number.is_connected()) {
            // TODO: Should flush here or return empty vector!!
            return m_current_value;
        }

        auto trigger = m_trigger.process();
        if (trigger.is_empty_like()) {
            // TODO: return empty vector!!
            return m_current_value;
        }

        auto note_number = m_note_number.process();
        auto velocity = m_velocity.process();
        auto channel = m_channel.process();

        auto num_voices = voice_count(trigger.size(), note_number.size(), velocity.size(), channel.size());

        if (num_voices != m_make_notes.size()) {
            m_make_notes.resize(num_voices);
        }

        trigger.adapted_to(num_voices);
        auto note_numbers = note_number.adapted_to(num_voices).as_type<NoteNumber>();
        auto velocities = velocity.adapted_to(num_voices).firsts_or(100u);
        auto channels = channel.adapted_to(num_voices).firsts_or(0u);

        auto output = Voices<Event>::zeros(num_voices);
        for (std::size_t i = 0; i < num_voices; ++i) {
            output[i] = m_make_notes[i].process(trigger[i], note_numbers[i], velocities[i], channels[i]);
        }

        m_current_value = std::move(output);
        return m_current_value;
    }


    void set_trigger(Node<Trigger>* trigger) { m_trigger = trigger; }

    void set_note_number(Node<Facet>* note_number) { m_note_number = note_number; }

    void set_velocity(Node<Facet>* velocity) { m_velocity = velocity; }

    void set_channel(Node<Facet>* channel) { m_channel = channel; }

    Socket<Trigger>& get_trigger() { return m_trigger; }

    Socket<Facet>& get_note_number() { return m_note_number; }

    Socket<Facet>& get_velocity() { return m_velocity; }

    Socket<Facet>& get_channel() { return m_channel; }

private:
    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_note_number;
    Socket<Facet>& m_velocity;
    Socket<Facet>& m_channel;

    MultiVoiced<MakeNote, Event> m_make_notes;

    Voices<Event> m_current_value = Voices<Event>::empty_like();
};


// ==============================================================================================

struct MakeNoteWrapper {
    using Keys = MakeNoteNode::Keys;

    ParameterHandler parameter_handler;

    Sequence<Trigger> trigger{param::properties::trigger, parameter_handler, Voices<Trigger>::empty_like()};
    Sequence<Facet, std::size_t> note_number{Keys::NOTE_NUMBER, parameter_handler, Voices<std::size_t>::singular(60)};
    Sequence<Facet, std::size_t> velocity{Keys::VELOCITY, parameter_handler, Voices<std::size_t>::singular(100)};
    Sequence<Facet, std::size_t> channel{Keys::CHANNEL, parameter_handler, Voices<std::size_t>::singular(true)};
    Sequence<Facet, bool> enabled{param::properties::enabled, parameter_handler, Voices<bool>::singular(true)};
    Variable<Facet, std::size_t> num_voices{param::properties::num_voices, parameter_handler, 0};

    MakeNoteNode make_note_node{Keys::CLASS_NAME
                                , parameter_handler
                                , &trigger
                                , &note_number
                                , &velocity
                                , &channel
                                , &enabled
                                , &num_voices};

};


} // namespace serialist

#endif //SERIALISTLOOPER_MAKE_NOTE_H
