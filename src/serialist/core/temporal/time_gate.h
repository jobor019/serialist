
#ifndef SERIALISTLOOPER_TIME_GATE_H
#define SERIALISTLOOPER_TIME_GATE_H

#include <optional>
#include "core/temporal/transport.h"
#include "core/types/time_point.h"

namespace serialist {

/**
 * Detects updates in time. Typically used to detect whether process() already has been called this particular cycle
 */
class TimeGate {
public:
    void push_time(const TimePoint& t) {
        m_time = t;
    }


    std::optional<TimePoint> pop_time() {
        if (m_time) {
            auto ret = m_time;
            m_time = std::nullopt;
            return ret;
        } else {
            return std::nullopt;
        }
    }


private:
    std::optional<TimePoint> m_time = std::nullopt;
};


// ==============================================================================================

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
            // Note: this will only be output the exact cycle the transport was paused. Most Nodes are not
            // interested in this, as NodeBase::is_enabled() already includes transport's running state
            v.append(TimeEvent::transport_paused);
        if (transport_resumed(t))
            // Note: this will only be output the exact cycle the transport was paused. Most Nodes are not
            // interested in this, as NodeBase::is_enabled() already includes transport's running state
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


// ==============================================================================================

enum class EnabledState {
    enabled
    , enabled_this_cycle
    , disabled_this_cycle
    , disabled_previous_cycle
    , disabled
};

class EnabledGate {
public:
    explicit EnabledGate(bool initial_enabled_state = false)
            : m_current_state(initial_enabled_state), m_previous_state(initial_enabled_state) {}


    EnabledState update(bool enabled) {
        auto state = process_state(enabled);
        m_previous_state = m_current_state;
        m_current_state = enabled;
        return state;
    }


    static bool enabled(EnabledState state) {
        return state == EnabledState::enabled
               || state == EnabledState::enabled_this_cycle;
    }

private:
    EnabledState process_state(bool enabled) const {
        if (enabled && m_current_state) {
            return EnabledState::enabled;
        } else if (enabled) {
            return EnabledState::enabled_this_cycle;
        } else if (m_current_state) {
            return EnabledState::disabled_this_cycle;
        } else if (m_previous_state) {
            return EnabledState::disabled_previous_cycle;
        } else {
            return EnabledState::disabled;
        }
    }

    bool m_current_state;
    bool m_previous_state;
};


} // namespace serialist

#endif //SERIALISTLOOPER_TIME_GATE_H
