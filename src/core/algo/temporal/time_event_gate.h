
#ifndef SERIALISTLOOPER_TIME_EVENT_GATE_H
#define SERIALISTLOOPER_TIME_EVENT_GATE_H

#include <optional>
#include "core/algo/temporal/transport.h"
#include "core/algo/temporal/time_point.h"

enum class TimeEvent {
    time_skip
    , transport_paused
    , transport_resumed
};


/**
 * Detects discrete events based on successive TimePoints.
 * Events will only be output once as they occur (e.g. transport_paused will be output at the first TimePoint where
 * this occurs, not continuously while the transport is paused)
 */
class TimeEventGate {
public:
    static constexpr double JUMP_THRESHOLD_TICKS = 1.0;   // quarter note

    explicit TimeEventGate(double jump_threshold = JUMP_THRESHOLD_TICKS) : m_jump_threshold(jump_threshold) {}

    Vec<TimeEvent> poll(const TimePoint& t) {
        auto v = Vec<TimeEvent>();
        if (transport_paused(t))
            v.append(TimeEvent::transport_paused);
        if (transport_resumed(t))
            v.append(TimeEvent::transport_resumed);
        if (time_skip_occurred(t))
            v.append(TimeEvent::time_skip);

        m_time = t;
        return v;
    }

    void reset() {
        m_time = std::nullopt;
    }

private:
    bool time_skip_occurred(const TimePoint& t) const {
        return m_time && std::abs(t.get_tick() - m_time->get_tick()) > m_jump_threshold;
    }

    bool transport_paused(const TimePoint& t) const {
        return m_time && !t.get_transport_running() && m_time->get_transport_running();
    }

    bool transport_resumed(const TimePoint& t) const {
        return m_time && t.get_transport_running() && !m_time->get_transport_running();
    }


    double m_jump_threshold;

    std::optional<TimePoint> m_time = std::nullopt;
};

#endif //SERIALISTLOOPER_TIME_EVENT_GATE_H
