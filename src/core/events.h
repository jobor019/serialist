

#ifndef SERIALIST_LOOPER_EVENTS_H
#define SERIALIST_LOOPER_EVENTS_H

#include <vector>
#include "transport.h"
#include "core/algo/voice/voices.h"

class Event {
public:
    Event() = default;
    virtual ~Event() = default;
    Event(const Event&) = default;
    Event& operator=(const Event&) = default;
    Event(Event&&) = default;
    Event& operator=(Event&&) = default;


    virtual double get_time() = 0;

};


// ==============================================================================================

class Trigger : public Event {
public:
    enum class Type {
        pulse
        , pulse_off
    };


    Trigger(double time, Type type, int id) : m_time(time), m_type(type), m_id(id) {}


    static bool contains(Voice<Trigger> voice, Type trigger_type) {
        auto it = std::find_if(voice.vector().begin()
                               , voice.vector().end()
                               , [&trigger_type](const auto& item) { return item.get_type() == trigger_type; });

        return it != voice.vector().end();
    }


    double get_time() override { return m_time; }


    int get_id() const { return m_id; }


    Type get_type() const { return m_type; }


private:
    double m_time;
    Type m_type;
    int m_id;
};


// ==============================================================================================

class MidiEvent : public Event {
public:
    MidiEvent(double time, int midi_cents, int velocity, int channel)
            : m_trigger_time(time), m_midi_cents(midi_cents), m_velocity(velocity), m_channel(channel) {}


    static MidiEvent note_on(double time, int midi_cents, int velocity, int channel) {
        return {time, midi_cents, velocity, channel};
    }


    static MidiEvent note_off(double time, int midi_cents, int channel) {
        return {time, midi_cents, 0, channel};
    }


    static std::pair<MidiEvent, MidiEvent> note(double onset, int midi_cents
                                                , int velocity, int channel, double duration) {
        return std::make_pair<MidiEvent, MidiEvent>({onset, midi_cents, velocity, channel}
                                                    , {onset + duration, midi_cents, 0, channel});
    }


    double get_time() override { return m_trigger_time; }


    int get_midi_cents() const { return m_midi_cents; }


    int get_note_number() const { return m_midi_cents / 100; }


    int get_velocity() const { return m_velocity; }


    int get_channel() const { return m_channel; }


    bool is_note_on() const { return m_velocity > 0; }


    bool matches(const MidiEvent& other) const {
        return matches(other.m_midi_cents, other.m_channel);
    }


    bool matches(int midi_cents, int channel) const {
        return m_midi_cents == midi_cents && m_channel == channel;
    }


    void print() const {
        std::cout << "MidiEvent(t=" << m_trigger_time
                  << ", cents=" << m_midi_cents
                  << ", vel=" << m_velocity
                  << ", ch=" << m_channel << ")\n";
    }


private:
    double m_trigger_time;

    int m_midi_cents;
    int m_velocity;
    int m_channel;
};

#endif //SERIALIST_LOOPER_EVENTS_H
