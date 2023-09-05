

#ifndef SERIALISTLOOPER_NOTE_SOURCE_H
#define SERIALISTLOOPER_NOTE_SOURCE_H

#include <vector>
#include "events.h"
#include "scheduler.h"
#include "io/renderers.h"
#include "generative.h"
#include "utils.h"
#include "parameter_policy.h"
#include "socket_policy.h"
#include "held_notes.h"
#include "socket_handler.h"


class NoteSource : public Root {
public:

    static const int HISTORY_LENGTH = 300;

    class NoteSourceKeys {
    public:
        static const inline std::string PULSE = "pulse";
        static const inline std::string PITCH = "pitch";
        static const inline std::string VELOCITY = "velocity";
        static const inline std::string CHANNEL = "channel";
        static const inline std::string ENABLED = "enabled";

        static const inline std::string CLASS_NAME = "notesource";
    };


    NoteSource(const std::string& id
               , ParameterHandler& parent
               , Node<Trigger>* trigger_pulse = nullptr
               , Node<Facet>* pitch = nullptr
               , Node<Facet>* velocity = nullptr
               , Node<Facet>* channel = nullptr
               , Node<Facet>* enabled = nullptr
               , Node<Facet>* num_voices = nullptr)
            : m_parameter_handler(id, parent)
              , m_socket_handler(m_parameter_handler)
              , m_trigger_pulse(m_socket_handler.create_socket(NoteSourceKeys::PULSE, trigger_pulse))
              , m_pitch(m_socket_handler.create_socket(NoteSourceKeys::PITCH, pitch))
              , m_velocity(m_socket_handler.create_socket(NoteSourceKeys::VELOCITY, velocity))
              , m_channel(m_socket_handler.create_socket(NoteSourceKeys::CHANNEL, channel))
              , m_enabled(m_socket_handler.create_socket(NoteSourceKeys::ENABLED, enabled))
              , m_num_voices(m_socket_handler.create_socket(ParameterKeys::NUM_VOICES, num_voices))
              , m_played_notes(HISTORY_LENGTH) {
        m_parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, NoteSourceKeys::CLASS_NAME);
    }

    void update_time(const TimePoint &t) override {
        m_time_point = t;
    }

    void process() override {
        if (!is_enabled() || !is_valid()) {
            if (m_previous_enabled_state) {
                for (auto& note_off: flush_all(m_time_point)) {
                    m_midi_renderer.render(note_off);
                }
            }
            return;
        }

        auto& t = m_time_point;

        auto voices = m_num_voices.process();

        auto trigger = m_trigger_pulse.process();
        auto pitch = m_pitch.process();
        auto velocity = m_velocity.process();
        auto channel = m_channel.process();

        auto num_voices = compute_voice_count(voices, trigger.size(), pitch.size(), velocity.size(), channel.size());

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


    void set_trigger_pulse(Node<Trigger>* pulse) { m_trigger_pulse = pulse; }


    void set_pitch(Node<Facet>* pitch) { m_pitch = pitch; }


    void set_velocity(Node<Facet>* velocity) { m_velocity = velocity; }


    void set_channel(Node<Facet>* channel) { m_channel = channel; }


    void set_enabled(Node<Facet>* enabled) { m_enabled = enabled; }


    void set_num_voices(Node<Facet>* num_voices) { m_num_voices = num_voices; }


    Socket<Trigger>& get_trigger_pulse() { return m_trigger_pulse; }


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

        std::vector<MidiEvent> midi_events;

        if (contains_pulse_off(triggers)) {
            auto note_offs = m_held_notes.at(voice_index).flush(t);
            midi_events.insert(midi_events.end(), note_offs.begin(), note_offs.end());
        }

        if (contains_pulse_on(triggers)) {
            auto note_ons = create_chord(t, midi_cents, velocities, channels);
            midi_events.insert(midi_events.end(), note_ons.begin(), note_ons.end());
            m_held_notes.at(voice_index).extend(note_ons);
        }

        return midi_events;
    }


    static std::vector<MidiEvent> create_chord(const TimePoint& t
                                               , const Voice<Facet>& midi_cents
                                               , const Voice<Facet>& velocities
                                               , const Voice<Facet>& channels) {
        if (midi_cents.empty() || velocities.empty() || channels.empty()) {
            return {};
        }

        auto num_notes = std::max(midi_cents.size(), channels.size());

        std::vector<int> mcs = midi_cents.adapted_to<int>(num_notes);
        std::vector<int> vs = velocities.adapted_to<int>(num_notes);
        std::vector<int> cs = channels.adapted_to<int>(num_notes);

        std::vector<MidiEvent> notes;

        for (std::size_t i = 0; i < num_notes; ++i) {
            notes.emplace_back(t.get_tick(), mcs.at(i), vs.at(i), cs.at(i));
        }

        return notes;


    }


    std::vector<MidiEvent> flush_all(const TimePoint& t) {
        std::vector<MidiEvent> note_offs;
        for (auto& held_notes_container: m_held_notes) {
            auto flushed = held_notes_container.flush(t);
            note_offs.insert(note_offs.end(), flushed.begin(), flushed.end());
        }
        return note_offs;
    }


    static bool contains_pulse_off(const Voice<Trigger>& triggers) {
        return contains(triggers, Trigger::Type::pulse_off);
    }


    static bool contains_pulse_on(const Voice<Trigger>& triggers) {
        return contains(triggers, Trigger::Type::pulse);
    }


    static bool contains(const Voice<Trigger>& triggers, Trigger::Type type) {
        return std::find_if(
                triggers.vector().begin()
                , triggers.vector().end()
                , [&type](const auto& trigger) { return trigger.get_type() == type; }
        ) != triggers.vector().end();
    }


    bool is_enabled() {
        // Unlike most other modules, NoteSource is disabled by default to avoid unnecessary output on creation
        return m_enabled.process(1).front_or(false);
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

    utils::LockingQueue<MidiEvent> m_played_notes;
    std::vector<HeldNotes> m_held_notes;

    bool m_previous_enabled_state = false;

    TimePoint m_time_point;


};


#endif //SERIALISTLOOPER_NOTE_SOURCE_H
