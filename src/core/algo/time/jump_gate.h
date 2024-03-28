
#ifndef SERIALISTLOOPER_JUMP_GATE_H
#define SERIALISTLOOPER_JUMP_GATE_H

#include <optional>
#include "core/algo/time/transport.h"

/**
 * Detect jumps in time
 */
class JumpGate {
public:
    static constexpr double JUMP_THRESHOLD_TICKS = 1.0;   // quarter note

    explicit JumpGate(double jump_threshold = JUMP_THRESHOLD_TICKS) : m_jump_threshold(jump_threshold) {}

    bool poll(const TimePoint& t) {
        if (!m_time) {
            return false; // first callback to object: no time has been set yet
        } else {
            return std::abs(t.get_tick() - m_time->get_tick()) > m_jump_threshold;
        }
    }

private:
    double m_jump_threshold;

    std::optional<TimePoint> m_time = std::nullopt;
};

#endif //SERIALISTLOOPER_JUMP_GATE_H
