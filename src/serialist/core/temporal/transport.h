

#ifndef SERIALIST_LOOPER_TRANSPORT_H
#define SERIALIST_LOOPER_TRANSPORT_H

#include <chrono>
#include <cmath>
#include "core/algo/fraction.h"
#include "core/utility/math.h"
#include "meter.h"
#include "core/temporal/time_point.h"

namespace serialist {

// ==============================================================================================

class Transport {
public:
    Transport() : Transport(TimePoint(), false) {}


    explicit Transport(bool active) : Transport(TimePoint::zero(), active) {}


    explicit Transport(const TimePoint& initial_time, bool active)
            : m_time_point(TimePoint{initial_time}.with_transport_running(active))
              , m_previous_update_time(std::chrono::system_clock::now()) {}


    void start() {
        m_time_point.with_transport_running(true);
        m_previous_update_time = std::chrono::system_clock::now();
    }


    void stop() {
        m_time_point.with_transport_running(false);
    }


    void set_tempo(double tempo) {
        m_time_point.with_tempo(tempo);
    }


    const TimePoint& update_time() {
        if (active()) {
            auto current_time = std::chrono::system_clock::now();
            auto time_delta = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    current_time - m_previous_update_time).count();
            m_time_point.increment(time_delta);
            m_previous_update_time = current_time;
        }

        return m_time_point;
    }

    bool active() const { return m_time_point.get_transport_running(); }


private:
    TimePoint m_time_point;

    std::chrono::time_point<std::chrono::system_clock> m_previous_update_time;


};

} // namespace serialist

#endif //SERIALIST_LOOPER_TRANSPORT_H
