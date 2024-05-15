

#ifndef SERIALIST_LOOPER_TRANSPORT_H
#define SERIALIST_LOOPER_TRANSPORT_H

#include <chrono>
#include <cmath>
#include "core/algo/fraction.h"
#include "core/utility/math.h"
#include "meter.h"
#include "time_point.h"


// ==============================================================================================


// ==============================================================================================

class Transport {
public:
    Transport() : Transport(TimePoint(), false) {}


    explicit Transport(bool active) : Transport(TimePoint(), active) {}


    explicit Transport(TimePoint&& initial_time, bool active)
            : m_time_point(initial_time)
              , m_active(active)
              , m_previous_update_time(std::chrono::system_clock::now()) {}


    void start() {
        m_active = true;
        m_previous_update_time = std::chrono::system_clock::now();
    }


    void stop() {
        m_active = false;
    }


    const TimePoint& update_time() {
        if (m_active) {
            auto current_time = std::chrono::system_clock::now();
            auto time_delta = std::chrono::duration_cast<std::chrono::nanoseconds>(
                    current_time - m_previous_update_time).count();
            m_time_point.increment(time_delta);
            m_previous_update_time = current_time;
        }

        return m_time_point;
    }


private:
    TimePoint m_time_point;

    bool m_active;
    std::chrono::time_point<std::chrono::system_clock> m_previous_update_time;


};

#endif //SERIALIST_LOOPER_TRANSPORT_H
