
#ifndef SERIALISTLOOPER_TIME_GATE_H
#define SERIALISTLOOPER_TIME_GATE_H

#include <optional>
#include "core/algo/time/transport.h"
#include "core/algo/time/time_point.h"

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

#endif //SERIALISTLOOPER_TIME_GATE_H
