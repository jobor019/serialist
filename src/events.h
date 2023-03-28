

#ifndef SERIALIST_LOOPER_EVENTS_H
#define SERIALIST_LOOPER_EVENTS_H

#include <vector>

class Event {
public:
    explicit Event(double time) : time(time) {}

    Event(const Event&) = default;

    Event& operator=(const Event&) = default;

    Event(Event&&) = default;

    Event& operator=(Event&&) = default;

    virtual ~Event() = default;

    [[nodiscard]] double get_time() const { return time; }

private:
    double time;
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
            : Event(time), midi_cents(midi_cents), velocity(velocity), channel(channel) {}


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


    [[nodiscard]] int get_midi_cents() const { return midi_cents; }

    [[nodiscard]] int get_note_number() const { return midi_cents / 100; }

    [[nodiscard]] int get_velocity() const { return velocity; }

    [[nodiscard]] int get_channel() const { return channel; }


private:
    int midi_cents;
    int velocity;
    int channel;
};

#endif //SERIALIST_LOOPER_EVENTS_H
