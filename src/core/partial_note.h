
#ifndef SERIALISTLOOPER_PARTIAL_NOTE_H
#define SERIALISTLOOPER_PARTIAL_NOTE_H

#include <optional>
#include "events.h"
#include "facet.h"


class PartialNote {
public:
    PartialNote(std::optional<Trigger> trigger
                , std::optional<Facet> note_number
                , std::optional<Facet> velocity
                , std::optional<Facet> channel
                , std::optional<Facet> duration)
            : m_trigger(trigger)
              , m_note_number(note_number)
              , m_velocity(velocity)
              , m_channel(channel)
              , m_duration(duration) {}

    const std::optional<Trigger>& get_trigger() const { return m_trigger; }
    const std::optional<Facet>& get_note_number() const { return m_note_number; }
    const std::optional<Facet>& get_velocity() const { return m_velocity; }
    const std::optional<Facet>& get_channel() const { return m_channel; }
    const std::optional<Facet>& get_duration() const { return m_duration; }

    void set_trigger(const std::optional<Trigger>& trigger) { m_trigger = trigger; }
    void set_note_number(const std::optional<Facet>& note_number) { m_note_number = note_number; }
    void set_velocity(const std::optional<Facet>& velocity) { m_velocity = velocity; }
    void set_channel(const std::optional<Facet>& channel) { m_channel = channel; }
    void set_duration(const std::optional<Facet>& duration) { m_duration = duration; }


private:
    std::optional<Trigger> m_trigger = std::nullopt;
    std::optional<Facet> m_note_number = std::nullopt;
    std::optional<Facet> m_velocity = std::nullopt;
    std::optional<Facet> m_channel = std::nullopt;
    std::optional<Facet> m_duration = std::nullopt;

};

#endif //SERIALISTLOOPER_PARTIAL_NOTE_H
