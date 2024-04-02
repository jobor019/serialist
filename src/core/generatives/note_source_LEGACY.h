

#ifndef SERIALISTLOOPER_NOTE_SOURCE_LEGACY_H
#define SERIALISTLOOPER_NOTE_SOURCE_LEGACY_H

#include <vector>
#include "core/algo/time/LEGACY_events.h"
#include "core/collections/scheduler.h"
#include "core/algo/pitch/notes.h"
#include "core/generative.h"
#include "core/utility/enums.h"
#include "core/param/parameter_policy.h"
#include "core/param/socket_policy.h"
#include "core/param/socket_handler.h"
#include "io/renderers.h"
#include "core/generatives/stereotypes/base_stereotypes.h"


// TODO: Lots of refactoring needed here: Separate out NoteSource from NoteSourceNode + update accordingly
class NoteSource : public Root {
public:


    static const int HISTORY_LENGTH = 300;

    class Keys {
    public:
        static const inline std::string PULSE = "pulse_on";
        static const inline std::string PITCH = "pitch";
        static const inline std::string VELOCITY = "velocity";
        static const inline std::string CHANNEL = "channel";
        static const inline std::string ENABLED = "enabled";

        static const inline std::string CLASS_NAME = "notesource";
    };


    NoteSource(const std::string& id
               , ParameterHandler& parent
               , Node<TriggerEvent>* trigger_pulse = nullptr
               , Node<Facet>* pitch = nullptr
               , Node<Facet>* velocity = nullptr
               , Node<Facet>* channel = nullptr
               , Node<Facet>* enabled = nullptr
               , Node<Facet>* num_voices = nullptr)
            : m_parameter_handler(id, parent)
              , m_socket_handler(m_parameter_handler)
              , m_trigger_pulse(m_socket_handler.create_socket(Keys::PULSE, trigger_pulse))
              , m_pitch(m_socket_handler.create_socket(Keys::PITCH, pitch))
              , m_velocity(m_socket_handler.create_socket(Keys::VELOCITY, velocity))
              , m_channel(m_socket_handler.create_socket(Keys::CHANNEL, channel))
              , m_enabled(m_socket_handler.create_socket(Keys::ENABLED, enabled))
              , m_num_voices(m_socket_handler.create_socket(ParameterKeys::NUM_VOICES, num_voices))
              , m_played_notes(HISTORY_LENGTH) {
        m_parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, Keys::CLASS_NAME);
    }

    void update_time(const TimePoint &t) override {
        m_time_point = t;
    }


    void process() override {
        // TODO: OUTDATED: USE MAKENOTE (and see X2. NoteSource in Obsidian)
        // TODO: OUTDATED: USE MAKENOTE (and see X2. NoteSource in Obsidian)
        // TODO: OUTDATED: USE MAKENOTE (and see X2. NoteSource in Obsidian)
        // TODO: OUTDATED: USE MAKENOTE (and see X2. NoteSource in Obsidian)
        // TODO: OUTDATED: USE MAKENOTE (and see X2. NoteSource in Obsidian)


        // TODO: Don't forget to handle legatissimo case!!!
        if (!is_enabled() || !is_valid()) {
            if (m_previous_enabled_state) {
                for (auto& note_off: flush_all(m_time_point)) {
                    m_midi_renderer.render(note_off);
                }
            }
            return;
        }

        auto& t = m_time_point;


        auto trigger = m_trigger_pulse.process();
        auto pitch = m_pitch.process();
        auto velocity = m_velocity.process();
        auto channel = m_channel.process();

        auto num_voices = GenerativeCommons::voice_count(m_num_voices, trigger.size(), pitch.size()
                                                         , velocity.size(), channel.size());

        if (m_held_notes.size() != num_voices) {
            for (auto& note_off: recompute_num_voices(t, num_voices)) {
                m_midi_renderer.render(note_off);
            }
        }

        if (trigger.is_empty_like())
            return;

        auto triggers = trigger.adapted_to(num_voices);
        auto pitches = pitch.adapted_to(num_voices);
        auto velocities = velocity.adapted_to(num_voices);
        auto channels = channel.adapted_to(num_voices);

        for (std::size_t i = 0; i < num_voices; ++i) {
            for (auto& event: process_voice(t, i, triggers.at(i), pitches.at(i), velocities.at(i), channels.at(i))) {
                m_midi_renderer.render(event);
                m_played_notes.push(event);
            }
        }
    }


    void disconnect_if(Generative& connected_to) override {
        m_socket_handler.disconnect_if(connected_to);
    }


    std::vector<Generative*> get_connected() override {
        return m_socket_handler.get_connected();
    }


    ParameterHandler& get_parameter_handler() override {
        return m_parameter_handler;
    }


    std::vector<MidiEvent> get_played_notes() {
        return m_played_notes.pop_all();
    }


    void set_trigger_pulse(Node<TriggerEvent>* pulse) { m_trigger_pulse = pulse; }


    void set_pitch(Node<Facet>* pitch) { m_pitch = pitch; }


    void set_velocity(Node<Facet>* velocity) { m_velocity = velocity; }


    void set_channel(Node<Facet>* channel) { m_channel = channel; }


    void set_enabled(Node<Facet>* enabled) { m_enabled = enabled; }


    void set_num_voices(Node<Facet>* num_voices) { m_num_voices = num_voices; }


    Socket<TriggerEvent>& get_trigger_pulse() { return m_trigger_pulse; }


    Socket<Facet>& get_pitch() { return m_pitch; }


    Socket<Facet>& get_velocity() { return m_velocity; }


    Socket<Facet>& get_channel() { return m_channel; }


    Socket<Facet>& get_enabled() { return m_enabled; }


    Socket<Facet>& get_num_voices() { return m_num_voices; }


    void set_midi_device(const std::string& device_name, bool override = true) {
        m_midi_renderer.initialize(device_name, override);
    }


private:
    [[nodiscard]] std::vector<MidiEvent> recompute_num_voices(const TimePoint& t, std::size_t num_voices) {
        auto surplus_count = static_cast<long>(m_held_notes.size() - num_voices);

        std::vector<MidiEvent> note_offs;

        if (surplus_count > 0) {
            // remove surplus voices, flushing any held notes
            for (std::size_t i = num_voices; i < static_cast<std::size_t>(surplus_count); ++i) {
                auto flushed = m_held_notes.at(i).flush(t);
                note_offs.insert(note_offs.end(), flushed.begin(), flushed.end());
            }

            m_held_notes.erase(m_held_notes.end() - surplus_count, m_held_notes.end());

        } else if (surplus_count < 0) {
            // insert empty HeldNotes objects for held notes
            m_held_notes.resize(m_held_notes.size() - static_cast<std::size_t>(surplus_count), {});
        }

        return note_offs;

    }


    [[nodiscard]] std::vector<MidiEvent> process_voice(const TimePoint& t
                                                       , std::size_t voice_index
                                                       , const Voice<Trigger>& triggers
                                                       , const Voice<Facet>& midi_cents
                                                       , const Voice<Facet>& velocities
                                                       , const Voice<Facet>& channels) {
        if (triggers.empty())
            return {};

        Vec<MidiEvent> midi_events;

        if (triggers.contains(Trigger::pulse_off)) {
            auto note_offs = m_held_notes.flush(voice_index);
            midi_events.extend(note_offs.as_type<MidiEvent>([&t](const auto& e) {
                return MidiEvent::note_off(t.get_tick(), e.note, e.channel);
            }));
        }

        if (triggers.contains(Trigger::pulse_on)) {
            auto note_ons = create_chord(t, midi_cents, velocities, channels);
            midi_events.insert(midi_events.end(), note_ons.begin(), note_ons.end());
            m_held_notes.at(voice_index).extend(note_ons);
        }

        return midi_events;
    }


    static Vec<MidiEvent> create_chord(const TimePoint& t
                                               , Voice<Facet>&& midi_cents
                                               , Voice<Facet>&& velocities
                                               , Voice<Facet>&& channels) {
        if (midi_cents.empty() || velocities.empty() || channels.empty()) {
            return {};
        }

        // Only midi_cent and channel: no need to broadcast notes based on velocity only
        auto num_notes = std::max(midi_cents.size(), channels.size());

        auto mcs = std::move(midi_cents).resize_fold(num_notes).as_type<int>();
        auto vs = std::move(velocities).resize_fold(num_notes).as_type<int>();
        auto cs = std::move(channels).resize_fold(num_notes).as_type<int>();

        auto notes = Vec<MidiEvent>::allocated(num_notes);

        for (std::size_t i = 0; i < num_notes; ++i) {
            notes.append({t.get_tick(), mcs[i], vs[i], cs[i]});
        }

        return notes;
    }


//    std::vector<MidiEvent> flush_all(const TimePoint& t) {
//        std::vector<MidiEvent> note_offs;
//        for (auto& held_notes_container: m_held_notes) {
//            auto flushed = held_notes_container.flush(t);
//            note_offs.insert(note_offs.end(), flushed.begin(), flushed.end());
//        }
//        return note_offs;
//    }


//    static bool contains_pulse_off(const Voice<TriggerEvent>& triggers) {
//        return contains(triggers, TriggerEvent::Mode::pulse_off);
//    }
//
//
//    static bool contains_pulse_on(const Voice<TriggerEvent>& triggers) {
//        return contains(triggers, TriggerEvent::Mode::pulse_on);
//    }
//
//
//    static bool contains(const Voice<TriggerEvent>& triggers, TriggerEvent::Mode type) {
//        return std::find_if(
//                triggers.vector().begin()
//                , triggers.vector().end()
//                , [&type](const auto& trigger) { return trigger.get_type() == type; }
//        ) != triggers.vector().end();
//    }


    bool is_enabled() {
        // Unlike most other modules, NoteSource is disabled by default to avoid unnecessary output on creation
        return m_enabled.process(1).first_or(false);
    }


    bool is_valid() {
        return m_trigger_pulse.is_connected()
               && m_pitch.is_connected()
               && m_velocity.is_connected()
               && m_channel.is_connected();
    }


    ParameterHandler m_parameter_handler;
    SocketHandler m_socket_handler;

    Scheduler<MidiEvent> m_scheduler;
    MidiRenderer m_midi_renderer;

    Socket<Trigger>& m_trigger_pulse;
    Socket<Facet>& m_pitch;
    Socket<Facet>& m_velocity;
    Socket<Facet>& m_channel;

    Socket<Facet>& m_enabled;
    Socket<Facet>& m_num_voices;

//    utils::LockingQueue<MidiEvent> m_played_notes;
    MultiVoiceHeldNotes m_held_notes;


    bool m_previous_enabled_state = false;

    TimePoint m_time_point;


};


#endif //SERIALISTLOOPER_NOTE_SOURCE_LEGACY_H
