#ifndef SERIALIST_LOOPER_TRANSPORT_H
#define SERIALIST_LOOPER_TRANSPORT_H

#include <chrono>
#include <cmath>
#include "core/types/fraction.h"
#include "core/utility/math.h"
#include "core/types/meter.h"
#include "core/types/time_point.h"


namespace serialist {
// ==============================================================================================

class Transport {
public:
    Transport() : Transport(TimePoint(), false) {}


    explicit Transport(bool active) : Transport(TimePoint::zero(), active) {}


    explicit Transport(const TimePoint& initial_time, bool active)
        : m_time_point(TimePoint{initial_time}.with_transport_running(active))
        , m_previous_update_time(std::chrono::system_clock::now()) {}


    const TimePoint& start() {
        m_time_point.with_transport_running(true);
        m_previous_update_time = std::chrono::system_clock::now();
        return m_time_point;
    }


    const TimePoint& pause() {
        return m_time_point.with_transport_running(false);
    }


    const TimePoint& reset() {
        auto new_time = TimePoint::zero()
                .with_meter(m_time_point.get_meter())
                .with_tempo(m_time_point.get_tempo())
                .with_transport_running(m_time_point.get_transport_running());
        m_time_point = new_time;
        return m_time_point;
    }


    const TimePoint& stop() {
        reset();
        return pause();
    }


    void set_tempo(double tempo) { m_time_point.with_tempo(tempo); }

    void set_next_meter(const std::optional<Meter>& meter) { m_next_meter = meter; }


    const TimePoint& update_time() {
        if (active()) {
            auto current_time = std::chrono::system_clock::now();
            auto delta_nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
                current_time - m_previous_update_time).count();

            if (m_next_meter) {
                if (m_time_point.increment_with_meter_change(delta_nanos, *m_next_meter)) {
                    m_next_meter = std::nullopt;
                }
            }
            else {
                m_time_point.increment(delta_nanos);
            }

            m_previous_update_time = current_time;
        }

        return m_time_point;
    }


    bool active() const { return m_time_point.get_transport_running(); }


    /**
     * @return current `TimePoint` without updating the time
     */
    const TimePoint& get_time() const {
        return m_time_point;
    }

private:
    TimePoint m_time_point;

    std::chrono::time_point<std::chrono::system_clock> m_previous_update_time;

    std::optional<Meter> m_next_meter = std::nullopt;
};
} // namespace serialist

#endif //SERIALIST_LOOPER_TRANSPORT_H
