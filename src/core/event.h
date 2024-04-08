
#ifndef SERIALISTLOOPER_EVENT_H
#define SERIALISTLOOPER_EVENT_H

#include "core/algo/pitch/notes.h"


struct MidiNoteEvent {
        std::size_t note_number;
        std::size_t velocity;
        std::size_t channel;
    };

class Event {
public:
    using EventType = std::variant<MidiNoteEvent>;

    explicit Event(const EventType& e) : m_event(e) {}

    template<typename T>
    bool is() const noexcept {
        return std::holds_alternative<T>(m_event);
    }


    template<typename T>
    T as() const {
        return std::get<T>(m_event);
    }

private:
    EventType m_event;

};

//using Event = std::variant<MidiNoteEvent>;


//class Event {
//public:
//    Event() = default;
//
//    virtual ~Event() = default;
//
//    Event(const Event&) = delete;
//
//    Event& operator=(const Event&) = delete;
//
//    Event(Event&&) noexcept = default;
//
//    Event& operator=(Event&&) noexcept = default;
//
//    virtual std::unique_ptr<Event> clone() const = 0;
//};
//
//
//// ==============================================================================================
//
//
//class MidiNoteEvent : public Event {
//public:
//    MidiNoteEvent(NoteNumber note_number, unsigned int velocity, unsigned int channel)
//            : m_note_number(note_number), m_velocity(velocity), m_channel(channel) {}
//
//    NoteNumber get_note_number() const { return m_note_number; }
//
//    unsigned int get_velocity() const { return m_velocity; }
//
//    unsigned int get_channel() const { return m_channel; }
//
//    std::unique_ptr<Event> clone() const override {
//        return std::make_unique<MidiNoteEvent>(m_note_number, m_velocity, m_channel);
//    }
//
//private:
//    NoteNumber m_note_number; // TODO: Change to MidiCents
//    unsigned int m_velocity;
//    unsigned int m_channel;
//};

#endif //SERIALISTLOOPER_EVENT_H
