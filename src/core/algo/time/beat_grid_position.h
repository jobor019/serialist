
#ifndef SERIALISTLOOPER_BEAT_POSITION_H
#define SERIALISTLOOPER_BEAT_POSITION_H

#include "relative_time.h"
#include "core/algo/time/time_point.h"

class BeatGridPosition {
public:

    RelativeTimePoint next(TimePoint& t) {
        auto rem = t % m_period;
        if (rem >= m_offset) {
            return t + (m_period - rem);
        }
        return t + m_offset - rem;


    }

    const RelativeDuration& get_period() const { return m_period; }
    const RelativeDuration& get_offset() const { return m_offset; }

private:
    RelativeDuration m_period;
    RelativeDuration m_offset;


}


#endif //SERIALISTLOOPER_BEAT_POSITION_H
