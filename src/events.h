

#ifndef SERIALIST_LOOPER_EVENTS_H
#define SERIALIST_LOOPER_EVENTS_H

#include <vector>

class Event {
public:
    explicit Event(double time) : m_time(time) {}

    Event(const Event&) = default;

    Event& operator=(const Event&) = default;

    Event(Event&&) = default;

    Event& operator=(Event&&) = default;

    virtual ~Event() = default;

    [[nodiscard]] double get_time() const { return m_time; }

private:
    double m_time;
};


// ==============================================================================================

class TriggerEvent : public Event {
public:
    // TODO: TRIGGER TIME VS TARGET TIME
    explicit TriggerEvent(double time) : Event(time) {}

};



// ==============================================================================================

class MidiEvent : public Event {
public:
    MidiEvent(double time, int midi_cents, int velocity, int channel)
            : Event(time), m_midi_cents(midi_cents), m_velocity(velocity), m_channel(channel) {}


    static MidiEvent note_on(double time, int midi_cents, int velocity, int channel) {
        return {time, midi_cents, velocity, channel};
    }


    static MidiEvent note_off(double time, int midi_cents, int channel) {
        return {time, midi_cents, 0, channel};
    }

    static std::pair<MidiEvent, MidiEvent> note(double onset
                                                , int midi_cents
                                                , int velocity
                                                , int channel
                                                , double duration) {
        return std::make_pair<MidiEvent, MidiEvent>({onset, midi_cents, velocity, channel}
                                                    , {onset + duration, midi_cents, 0, channel});
    }


    [[nodiscard]] int get_midi_cents() const { return m_midi_cents; }

    [[nodiscard]] int get_note_number() const { return m_midi_cents / 100; }

    [[nodiscard]] int get_velocity() const { return m_velocity; }

    [[nodiscard]] int get_channel() const { return m_channel; }


private:
    int m_midi_cents;
    int m_velocity;
    int m_channel;
};

#endif //SERIALIST_LOOPER_EVENTS_H
