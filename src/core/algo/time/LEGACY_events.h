

#ifndef SERIALIST_LOOPER_EVENTS_H
#define SERIALIST_LOOPER_EVENTS_H

#include <vector>
#include "core/algo/time/transport.h"
#include "core/algo/time/trigger.h"
#include "core/collections/voices.h"


//class Event {
//public:
//    Event() = default;
//    virtual ~Event() = default;
//    Event(const Event&) = default;
//    Event& operator=(const Event&) = default;
//    Event(Event&&) = default;
//    Event& operator=(Event&&) = default;
//
//
//    virtual double get_time() = 0;
//
//};
//
//
//// ==============================================================================================
//
//class TriggerEvent : public Event {
//public:
//    TriggerEvent(double time, Trigger trigger) : m_time(time), m_trigger(trigger) {}
//
//
//
//    double get_time() override { return m_time; }
//
//
//    Trigger get_trigger() const { return m_trigger; }
//
//
//private:
//    double m_time;
//    Trigger m_trigger;
//};
//
//
//// ==============================================================================================
//
//class MidiEvent : public Event {
//public:
//    MidiEvent(double time, unsigned int midi_cents, unsigned int velocity, unsigned int channel)
//            : m_trigger_time(time), m_midi_cents(midi_cents), m_velocity(velocity), m_channel(channel) {}
//
//
//    static MidiEvent note_on(double time, unsigned int midi_cents, int velocity, int channel) {
//        return {time, midi_cents, velocity, channel};
//    }
//
//
//    static MidiEvent note_off(double time, unsigned int midi_cents, unsigned int channel) {
//        return {time, midi_cents, 0, channel};
//    }
//
//
//    static std::pair<MidiEvent, MidiEvent> note(double onset, int midi_cents
//                                                , int velocity, int channel, double duration) {
//        return std::make_pair<MidiEvent, MidiEvent>({onset, midi_cents, velocity, channel}
//                                                    , {onset + duration, midi_cents, 0, channel});
//    }
//
//
//    double get_time() override { return m_trigger_time; }
//
//
//    int get_midi_cents() const { return m_midi_cents; }
//
//
//    int get_note_number() const { return m_midi_cents / 100; }
//
//
//    int get_velocity() const { return m_velocity; }
//
//
//    int get_channel() const { return m_channel; }
//
//
//    bool is_note_on() const { return m_velocity > 0; }
//
//
//    bool matches(const MidiEvent& other) const {
//        return matches(other.m_midi_cents, other.m_channel);
//    }
//
//
//    bool matches(int midi_cents, int channel) const {
//        return m_midi_cents == midi_cents && m_channel == channel;
//    }
//
//
//    void print() const {
//        std::cout << "MidiEvent(t=" << m_trigger_time
//                  << ", cents=" << m_midi_cents
//                  << ", vel=" << m_velocity
//                  << ", ch=" << m_channel << ")\n";
//    }
//
//
//private:
//    double m_trigger_time;
//
//    int m_midi_cents;
//    int m_velocity;
//    int m_channel;
//};

#endif //SERIALIST_LOOPER_EVENTS_H
