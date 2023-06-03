

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

    static const int HISTORY_LENGTH = 50;


    MidiNoteSource(const std::string& id
                   , ParameterHandler& parent
                   , Node<float>* onset = nullptr
                   , Node<float>* duration = nullptr
                   , Node<int>* pitch = nullptr
                   , Node<int>* velocity = nullptr
                   , Node<int>* channel = nullptr
                   , Node<bool>* enabled = nullptr)
            : Source(id, parent)
              , m_onset("onset", *this, onset)
              , m_duration("duration", *this, duration)
              , m_pitch("pitch", *this, pitch)
              , m_velocity("velocity", *this, velocity)
              , m_channel("channel", *this, channel)
              , m_enabled("enabled", *this, enabled)
              , m_played_notes(HISTORY_LENGTH) {
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

                std::cout << "#################### Trigger: " << event->get_time() << "\n";

            } else if (auto note_event = dynamic_cast<MidiEvent*>(event.get())) {
                m_midi_renderer.render(note_event);
                m_played_notes.push(*note_event);

                std::cout << "note:    @" << note_event->get_time()
                          << " (" << note_event->get_note_number()
                          << ", " << note_event->get_velocity()
                          << ", " << note_event->get_channel() << ")\n";

            } else {
                std::cout << "unknown event type\n";
            }
        }
    }


    std::vector<Generative*> get_connected() override {
        return collect_connected(m_onset.get_connected()
                                 , m_duration.get_connected()
                                 , m_pitch.get_connected()
                                 , m_velocity.get_connected()
                                 , m_channel.get_connected());
    }


    std::vector<MidiEvent> get_played_notes() {
        return m_played_notes.pop_all();
    }


    void set_onset(Node<float>* onset) { m_onset = onset; }


    void set_duration(Node<float>* duration) { m_duration = duration; }


    void set_pitch(Node<int>* pitch) { m_pitch = pitch; }


    void set_velocity(Node<int>* velocity) { m_velocity = velocity; }


    void set_channel(Node<int>* channel) { m_channel = channel; }


    void set_enabled(Node<bool>* enabled) { m_enabled = enabled; }


    void set_midi_device(const std::string& device_name, bool override = true) {
        m_midi_renderer.initialize(device_name, override);
    }


private:
    std::vector<std::unique_ptr<Event>> new_event(const TimePoint& t) {
        std::cout << "new event\n";
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
                                    , note_pitch.at(0)
                                    , note_velocity.at(0)
                                    , note_channel.at(0)
                                    , next_onset.at(0) * note_duration.at(0));
        events.emplace_back(std::make_unique<MidiEvent>(note.first));
        events.emplace_back(std::make_unique<MidiEvent>(note.second));
        events.emplace_back(std::make_unique<TriggerEvent>(next_onset.at(0) + t.get_tick()));

        return events;
    }


    bool is_enabled(const TimePoint& t) {
        return m_enabled.process_or(t, false);
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


    Scheduler m_scheduler;
    MidiRenderer m_midi_renderer;

    Socket<float> m_onset;
    Socket<float> m_duration;
    Socket<int> m_pitch;
    Socket<int> m_velocity;
    Socket<int> m_channel;

    Socket<bool> m_enabled;

    utils::LockingQueue<MidiEvent> m_played_notes;


};


#endif //SERIALISTLOOPER_SOURCE_H
