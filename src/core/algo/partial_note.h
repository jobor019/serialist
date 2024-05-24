//
//#ifndef SERIALISTLOOPER_PARTIAL_NOTE_H
//#define SERIALISTLOOPER_PARTIAL_NOTE_H
//
//#include <optional>
//#include "core/algo/temporal/LEGACY_events.h"
//#include "core/algo/facet.h"
//
//
//class PartialNote {
//public:
//    explicit PartialNote(const std::optional<Trigger>& trigger = std::nullopt
//                         , const std::optional<Facet>& note_number = std::nullopt
//                         , const std::optional<Facet>& velocity = std::nullopt
//                         , const std::optional<Facet>& channel = std::nullopt
//                         , const std::optional<Facet>& duration = std::nullopt)
//            : m_trigger(trigger)
//              , m_note_number(note_number)
//              , m_velocity(velocity)
//              , m_channel(channel)
//              , m_duration(duration) {}
//
//
//    static PartialNote from_trigger(const Trigger& trig) { return PartialNote({trig}); }
//
//
//    static PartialNote from_note_on(const Facet& note) { return PartialNote(Trigger::pulse_on, note); }
//
//
//    static PartialNote from_note_on(NoteNumber note) { return PartialNote(Trigger::pulse_on, Facet(note)); }
//
//
//    static PartialNote from_note_off(const Facet& note) { return PartialNote(Trigger::pulse_off, note); }
//
//
//    static PartialNote from_note_off(NoteNumber note) { return PartialNote(Trigger::pulse_off, Facet(note)); }
//
//
//    static PartialNote from_velocity(const Facet& vel) { return PartialNote({}, {}, vel); }
//
//
//    const std::optional<Trigger>& get_trigger() const { return m_trigger; }
//
//
//    const std::optional<Facet>& get_note_number() const { return m_note_number; }
//
//
//    const std::optional<Facet>& get_velocity() const { return m_velocity; }
//
//
//    const std::optional<Facet>& get_channel() const { return m_channel; }
//
//
//    const std::optional<Facet>& get_duration() const { return m_duration; }
//
//
//    void set_trigger(const std::optional<Trigger>& trigger) { m_trigger = trigger; }
//
//
//    void set_note_number(const std::optional<Facet>& note_number) { m_note_number = note_number; }
//
//
//    void set_velocity(const std::optional<Facet>& velocity) { m_velocity = velocity; }
//
//
//    void set_channel(const std::optional<Facet>& channel) { m_channel = channel; }
//
//
//    void set_duration(const std::optional<Facet>& duration) { m_duration = duration; }
//
//
//private:
//    std::optional<Trigger> m_trigger;
//    std::optional<Facet> m_note_number;
//    std::optional<Facet> m_velocity;
//    std::optional<Facet> m_channel;
//    std::optional<Facet> m_duration;
//
//};
//
//#endif //SERIALISTLOOPER_PARTIAL_NOTE_H
