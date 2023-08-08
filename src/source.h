

#ifndef SERIALISTLOOPER_SOURCE_H
#define SERIALISTLOOPER_SOURCE_H

#include <vector>
#include "events.h"
#include "scheduler.h"
#include "io/renderers.h"
#include "generative.h"
#include "utils.h"
#include "parameter_policy.h"
#include "socket_policy.h"


class MidiNoteSource : public Source {
public:

    static const int HISTORY_LENGTH = 300;

    class NoteSourceKeys {
    public:
        static const inline std::string ONSET = "onset";
        static const inline std::string DURATION = "duration";
        static const inline std::string PITCH = "pitch";
        static const inline std::string VELOCITY = "velocity";
        static const inline std::string CHANNEL = "channel";
        static const inline std::string ENABLED = "enabled";

        static const inline std::string CLASS_NAME = "notesource";
    };


    MidiNoteSource(const std::string& id
                   , ParameterHandler& parent
                   , Node<Facet>* onset = nullptr
                   , Node<Facet>* duration = nullptr
                   , Node<Facet>* pitch = nullptr
                   , Node<Facet>* velocity = nullptr
                   , Node<Facet>* channel = nullptr
                   , Node<Facet>* enabled = nullptr
                   , Node<Facet>* num_voices = nullptr)
            : m_parameter_handler(id, parent)
              , m_socket_handler("", m_parameter_handler, ParameterKeys::GENERATIVE_SOCKETS_TREE)
              , m_onset(NoteSourceKeys::ONSET, m_socket_handler, onset)
              , m_duration(NoteSourceKeys::DURATION, m_socket_handler, duration)
              , m_pitch(NoteSourceKeys::PITCH, m_socket_handler, pitch)
              , m_velocity(NoteSourceKeys::VELOCITY, m_socket_handler, velocity)
              , m_channel(NoteSourceKeys::CHANNEL, m_socket_handler, channel)
              , m_enabled(NoteSourceKeys::ENABLED, m_socket_handler, enabled)
              , m_num_voices(ParameterKeys::NUM_VOICES, m_socket_handler, num_voices)
              , m_played_notes(HISTORY_LENGTH) {
        m_parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, NoteSourceKeys::CLASS_NAME);
    }


    void process(const TimePoint& time) override {
        if (!m_scheduler.has_trigger() || !m_midi_renderer.is_initialized()) {
            queue_trigger_at_next_tick(time);
            return;
        }

        if (!is_enabled(time))
            return;


        auto events = m_scheduler.poll(time);

        for (auto& event: events) {
            if (dynamic_cast<TriggerEvent*>(event.get())) {
                // TODO: Should use trigger's time to avoid drifting: how to handle time sig / tempo?
                m_scheduler.add_events(new_event(time));

//                std::cout << "#################### Trigger: " << event->get_time() << "\n";

            } else if (auto note_event = dynamic_cast<MidiEvent*>(event.get())) {
                m_midi_renderer.render(note_event);
                m_played_notes.push(*note_event);

//                std::cout << "note:    @" << note_event->get_time()
//                          << " (" << note_event->get_note_number()
//                          << ", " << note_event->get_velocity()
//                          << ", " << note_event->get_channel() << ")\n";

            } else {
                std::cout << "unknown event type\n";
            }
        }
    }


    void disconnect_if(Generative& connected_to) override {
        m_onset.disconnect_if(connected_to);
        m_duration.disconnect_if(connected_to);
        m_pitch.disconnect_if(connected_to);
        m_velocity.disconnect_if(connected_to);
        m_channel.disconnect_if(connected_to);
        m_onset.disconnect_if(connected_to);
        m_enabled.disconnect_if(connected_to);
        m_num_voices.disconnect_if(connected_to);
    }


    std::vector<Generative*> get_connected() override {
        return collect_connected(m_onset.get_connected()
                                 , m_duration.get_connected()
                                 , m_pitch.get_connected()
                                 , m_velocity.get_connected()
                                 , m_channel.get_connected()
                                 , m_enabled.get_connected()
                                 , m_num_voices.get_connected());
    }


    ParameterHandler& get_parameter_handler() override {
        return m_parameter_handler;
    }


    std::vector<MidiEvent> get_played_notes() {
        return m_played_notes.pop_all();
    }


    void set_onset(Node<Facet>* onset) { m_onset = onset; }
    void set_duration(Node<Facet>* duration) { m_duration = duration; }
    void set_pitch(Node<Facet>* pitch) { m_pitch = pitch; }
    void set_velocity(Node<Facet>* velocity) { m_velocity = velocity; }
    void set_channel(Node<Facet>* channel) { m_channel = channel; }
    void set_enabled(Node<Facet>* enabled) { m_enabled = enabled; }
    void set_num_voices(Node<Facet>* num_voices) { m_num_voices = num_voices; }


    Socket<Facet>& get_onset() { return m_onset; }
    Socket<Facet>& get_duration() { return m_duration; }
    Socket<Facet>& get_pitch() { return m_pitch; }
    Socket<Facet>& get_velocity() { return m_velocity; }
    Socket<Facet>& get_channel() { return m_channel; }
    Socket<Facet>& get_enabled() { return m_enabled; }
    Socket<Facet>& get_num_voices() { return m_num_voices; }


    void set_midi_device(const std::string& device_name, bool override = true) {
        m_midi_renderer.initialize(device_name, override);
    }


private:
    std::vector<std::unique_ptr<Event>> new_event(const TimePoint& t) {
//        std::cout << "new event\n";
        if (!is_valid()) {
            std::cout << "invalid\n";
            return default_retrigger(t);
        }

        auto note_pitch = m_pitch.process(t);
        auto note_velocity = m_velocity.process(t);
        auto note_duration = m_duration.process(t);
        auto note_channel = m_channel.process(t);
        auto next_onset = m_onset.process(t);

        std::vector<std::unique_ptr<Event>> events;

        if (note_pitch.empty() || note_velocity.empty() || note_duration.empty() ||
            note_channel.empty() || next_onset.empty()) {
            // if any is missing: ignore all and requeue trigger
            return default_retrigger(t);
        }

        // TODO: Handle vector properly rather than just using the first element
        auto note = MidiEvent::note(t.get_tick()
                                    , static_cast<int>(note_pitch.at(0))
                                    , static_cast<int>(note_velocity.at(0))
                                    , static_cast<int>(note_channel.at(0))
                                    , next_onset.at(0).get() * note_duration.at(0).get());
        events.emplace_back(std::make_unique<MidiEvent>(note.first));
        events.emplace_back(std::make_unique<MidiEvent>(note.second));
        events.emplace_back(std::make_unique<TriggerEvent>(next_onset.at(0).get() + t.get_tick()));

        return events;
    }


    bool is_enabled(const TimePoint& t) {
        return m_enabled.process(t, 1).front_or(false);
    }


    bool is_valid() {
        return m_onset.is_connected()
               && m_duration.is_connected()
               && m_pitch.is_connected()
               && m_velocity.is_connected()
               && m_channel.is_connected();
    }


    void queue_trigger_at_next_tick(const TimePoint& t) {
        std::cout << "requeueing at " << (std::floor(t.get_tick() + 1)) << "\n";
        m_scheduler.add_event(std::make_unique<TriggerEvent>(std::floor(t.get_tick() + 1)));
    }


    static std::vector<std::unique_ptr<Event>> default_retrigger(const TimePoint& t) {
        std::vector<std::unique_ptr<Event>> events;
        events.emplace_back(std::make_unique<TriggerEvent>(t.get_tick() + 1));
        return events;
    }


    ParameterHandler m_parameter_handler;
    SocketHandler m_socket_handler;

    Scheduler m_scheduler;
    MidiRenderer m_midi_renderer;

    Socket<Facet> m_onset;
    Socket<Facet> m_duration;
    Socket<Facet> m_pitch;
    Socket<Facet> m_velocity;
    Socket<Facet> m_channel;

    Socket<Facet> m_enabled;
    Socket<Facet> m_num_voices;

    utils::LockingQueue<MidiEvent> m_played_notes;


};


#endif //SERIALISTLOOPER_SOURCE_H
