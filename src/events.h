

#ifndef SERIALIST_LOOPER_EVENTS_H
#define SERIALIST_LOOPER_EVENTS_H


class Event {
public:
    explicit Event(float time) : time(time) {}

    virtual ~Event() = default;


    [[nodiscard]] float get_time() const { return time; }

private:
    float time;
};


// ==============================================================================================

class TriggerEvent : public Event {
public:
    // TODO: TRIGGER TIME VS TARGET TIME
    explicit TriggerEvent(float time) : Event(time) {}

    ~TriggerEvent() override = default;

};



// ==============================================================================================

class MidiEvent : public Event {
public:
    MidiEvent(float time, int midi_cents, int velocity, int channel)
            : Event(time), midi_cents(midi_cents), velocity(velocity), channel(channel) {}


    ~MidiEvent() override = default;


    static MidiEvent note_on(float time, int midi_cents, int velocity, int channel) {
        return {time, midi_cents, velocity, channel};
    }


    static MidiEvent note_off(float time, int midi_cents, int channel) {
        return {time, midi_cents, 0, channel};
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
