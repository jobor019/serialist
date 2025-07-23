#ifndef SERIALISTLOOPER_TIME_POINT_H
#define SERIALISTLOOPER_TIME_POINT_H

#include <cassert>
#include "core/types/domain_type.h"
#include "meter.h"
#include "core/exceptions.h"

namespace serialist {

// ==============================================================================================

class DomainTimePoint;
class DomainDuration;

// ==============================================================================================

class TimePoint {
public:
    explicit TimePoint(double tick = 0.0
                       , double tempo = 120.0
                       , std::optional<double> absolute_beat = std::nullopt
                       , std::optional<double> relative_beat = std::nullopt
                       , std::optional<double> bar = std::nullopt
                       , Meter meter = Meter()
                       , bool transport_running = true)
        : m_tick(tick)
        , m_tempo(tempo)
        , m_absolute_beat(absolute_beat.value_or(meter.ticks2beats(tick)))
        , m_relative_beat(relative_beat.value_or(meter.ticks2bars_beats(tick).second))
        , m_bar(bar.value_or(meter.ticks2bars(tick)))
        , m_meter(meter)
        , m_transport_running(transport_running) {
        assert(tempo > 0.0);
    }


    static TimePoint zero() { return TimePoint(); }

    /**
     * Typically used at construction time. Rewrites absolute/relative beat & bar in relation to tick.
     * To change meter of running time point, use `increment_with_meter_change` */
    TimePoint& with_meter(const Meter& meter) {
        m_meter = meter;
        m_absolute_beat = meter.ticks2beats(m_tick);
        m_relative_beat = meter.ticks2bars_beats(m_tick).second;
        m_bar = meter.ticks2bars(m_tick);
        return *this;
    }


    TimePoint& with_tempo(double tempo) {
        assert(tempo > 0.0);
        m_tempo = tempo;
        return *this;
    }

    TimePoint& with_transport_running(bool transport_running) {
        m_transport_running = transport_running;
        return *this;
    }


    // double arithmetic operators
    TimePoint operator+(double tick_increment) const;
    TimePoint operator-(double tick_decrement) const;
    TimePoint& operator+=(double tick_increment);
    TimePoint& operator-=(double tick_decrement);

    // DTP arithmetic operators
    DomainDuration operator-(const DomainTimePoint& dtp) const;

    // DD arithmetic operators
    DomainTimePoint operator+(const DomainDuration& duration) const;
    DomainTimePoint operator-(const DomainDuration& duration) const;

    /** utility increment function. Note that this means that tp + dd and tp += dd yields different return types! */
    TimePoint& operator+=(const DomainDuration& duration);

    // TP comparison operators
    bool operator<(const TimePoint& other) const { return m_tick < other.m_tick; }
    bool operator<=(const TimePoint& other) const { return m_tick <= other.m_tick; }
    bool operator>(const TimePoint& other) const { return m_tick > other.m_tick; }
    bool operator>=(const TimePoint& other) const { return m_tick >= other.m_tick; }

    // DTP comparison operators
    bool operator<(const DomainTimePoint& other) const;
    bool operator<=(const DomainTimePoint& other) const;
    bool operator>(const DomainTimePoint& other) const;
    bool operator>=(const DomainTimePoint& other) const;

    explicit operator std::string() const { return to_string(); }


    friend std::ostream& operator<<(std::ostream& os, const TimePoint& obj) {
        os << obj.to_string();
        return os;
    }


    void increment(int64_t delta_nanos);
    void increment(double tick_increment);
    void increment(const DomainDuration& delta);

    /**
     * Increment the `TimePoint` by `tick_increment`. If the increment results in a bar change, change the meter to
     * `new_meter` at the start of the bar and return true, otherwise keep old meter and return false.
     */
    bool increment_with_meter_change(int64_t delta_nanos, const Meter& new_meter);
    bool increment_with_meter_change(double tick_increment, const Meter& new_meter);
    bool increment_with_meter_change(const DomainDuration& delta, const Meter& new_meter);


    bool try_set_meter(const Meter& new_meter) {
        if (is_at_barline()) {
            m_meter = new_meter;
            return true;
        }
        return false;
    }


    TimePoint incremented(double tick_increment) const;
    TimePoint incremented(const DomainDuration& duration) const;
    std::pair<TimePoint, bool> incremented_with_meter_change(double tick_increment, const Meter& new_meter) const;

    double ticks_to_next_bar() const { return m_meter.bars2ticks(std::ceil(m_bar) - m_bar); }
    bool is_at_barline() const { return utils::equals(utils::modulo(m_bar, 1.0), 0.0); }


    std::size_t next_bar() const {
        return is_at_barline()
                   ? static_cast<std::size_t>(std::floor(m_bar))
                   : static_cast<std::size_t>(std::ceil(m_bar));
    }


    double distance_to(double time_value, DomainType type) const;


    double get_tick() const { return m_tick; }
    double get_tempo() const { return m_tempo; }
    double get_relative_beat() const { return m_relative_beat; }
    double get_absolute_beat() const { return m_absolute_beat; }
    double get_bar() const { return m_bar; }
    const Meter& get_meter() const { return m_meter; }
    bool get_transport_running() const { return m_transport_running; }


    double get(DomainType type) const {
        switch (type) {
            case DomainType::ticks:
                return m_tick;
            case DomainType::beats:
                return m_absolute_beat;
            case DomainType::bars:
                return m_bar;
        }
        throw std::runtime_error("Invalid type");
    }


    std::string to_string(std::optional<std::size_t> num_decimals = std::nullopt) const {
        std::stringstream ss;

        if (num_decimals) ss << std::fixed << std::setprecision(static_cast<int>(*num_decimals));

        ss << "TimePoint(tick=" << m_tick
                << ", abs_beat=" << m_absolute_beat
                << ", rel_beat=" << m_relative_beat
                << ", bar=" << m_bar
                << ", tempo=" << m_tempo
                << ", meter=" << m_meter.to_string() << ")";

        return ss.str();
    }

    double nanos2ticks(int64_t delta_nanos) const { return static_cast<double>(delta_nanos) * 1e-9 * m_tempo / 60.0; }

private:
    double m_tick;          // tick number since start, where 1.0 ticks equals 1 quarter note (indep. of meter)
    double m_tempo;
    double m_absolute_beat; // beat number since start (fraction corresponds to position in current beat)
    double m_relative_beat; // beat number since the last bar (fraction corresponds to position in current beat)
    double m_bar;           // bar number since start (fraction corresponds to position in current bar)
    Meter m_meter;
    bool m_transport_running;
};


// ==============================================================================================


class DomainTimePoint {
public:
    DomainTimePoint(double value, DomainType type) : m_value(value)
                                                   , m_type(type) {}


    static DomainTimePoint from_time_point(const TimePoint& t, DomainType type) { return {t.get(type), type}; }
    static DomainTimePoint from_domain_duration(const DomainDuration& duration);
    static DomainTimePoint zero() { return {0.0, DomainType::ticks}; }

    static DomainTimePoint ticks(double value) { return {value, DomainType::ticks}; }
    static DomainTimePoint bars(double value) { return {value, DomainType::bars}; }
    static DomainTimePoint beats(double value) { return {value, DomainType::beats}; }

    // double arithmetic operators
    DomainTimePoint operator+(double other) const { return {m_value + other, m_type}; }
    DomainTimePoint operator-(double other) const { return {m_value - other, m_type}; }
    DomainTimePoint operator*(double other) const { return {m_value * other, m_type}; }

    // TP arithmetic operators
    DomainDuration operator-(const TimePoint& other) const;

    // DTP arithmetic operators
    /** throws TimeTypeError if `other` has a different type */
    DomainDuration operator-(const DomainTimePoint& other) const;

    // DD arithmetic operators
    /** throws TimeTypeError if `duration` has a different type */
    DomainTimePoint operator+(const DomainDuration& duration) const;
    /** throws TimeTypeError if `duration` has a different type */
    DomainTimePoint operator-(const DomainDuration& duration) const;

    bool operator<(const TimePoint& t) const { return m_value < t.get(m_type); }
    bool operator<=(const TimePoint& t) const { return m_value <= t.get(m_type); }
    bool operator>(const TimePoint& t) const { return m_value > t.get(m_type); }
    bool operator>=(const TimePoint& t) const { return m_value >= t.get(m_type); }


    bool supports(const DomainTimePoint& t) const {
        return t.get_type() == m_type;
    }


    bool supports(const DomainDuration& other) const;


    DomainTimePoint& operator+=(double other) {
        m_value += other;
        return *this;
    }


    DomainTimePoint& operator-=(double other) {
        m_value -= other;
        return *this;
    }


    std::string to_string(std::optional<std::size_t> num_decimals = std::nullopt) const {
        std::stringstream ss;

        ss << "DomainTimePoint(" << domain_type_to_string(m_type) << "=";
        if (num_decimals) {
            ss << std::fixed << std::setprecision(static_cast<int>(*num_decimals));
        }
        ss << m_value << ")";

        return ss.str();
    }


    std::string to_string_compact(std::optional<std::size_t> num_decimals = std::nullopt) const {
        std::stringstream ss;
        if (num_decimals) {
            ss << std::fixed << std::setprecision(static_cast<int>(*num_decimals));
        }
        ss << m_value << " " << domain_type_to_string(m_type);
        return ss.str();
    }


    friend std::ostream& operator<<(std::ostream& os, const DomainTimePoint& obj) {
        os << obj.to_string();
        return os;
    }


    /** @throws TimeDomainError if `other` has a different type */
    static DomainTimePoint min(const DomainTimePoint& a, const DomainTimePoint& b) {
        if (a.m_type != b.m_type)
            throw TimeDomainError("DomainTypes are incompatible");

        return {std::min(a.m_value, b.m_value), a.m_type};
    }


    /** @throws TimeDomainError if `other` has a different type */
    static DomainTimePoint max(const DomainTimePoint& a, const DomainTimePoint& b) {
        if (a.m_type != b.m_type)
            throw TimeDomainError("DomainTypes are incompatible");

        return {std::max(a.m_value, b.m_value), a.m_type};
    }


    // Inline declaration due to dependency of DomainConverter
    DomainTimePoint as_type(DomainType target_type, const TimePoint& last_transport_tp) const;


    bool elapsed(const TimePoint& current_time) const { return current_time.get(m_type) >= m_value; }

    DomainType get_type() const { return m_type; }
    double get_value() const { return m_value; }

private:
    double m_value;
    DomainType m_type;
};


// ==============================================================================================

class DomainDuration {
public:
    explicit DomainDuration(double value = 1.0, DomainType type = DomainType::ticks) : m_value(value)
        , m_type(type) {}


    static DomainDuration from_domain_time_point(const DomainTimePoint& dtp) {
        return DomainDuration{dtp.get_value(), dtp.get_type()};
    }


    static DomainDuration ticks(double value) { return DomainDuration{value, DomainType::ticks}; }
    static DomainDuration beats(double value) { return DomainDuration{value, DomainType::beats}; }
    static DomainDuration bars(double value) { return DomainDuration{value, DomainType::bars}; }


    static DomainDuration distance(const TimePoint& from, const DomainTimePoint& to) {
        auto t = to.get_type();
        return DomainDuration(to.get_value() - from.get(t), t);
    }


    /** throws TimeTypeError if `other` has a different type */
    static DomainDuration distance(const DomainTimePoint& from, const DomainTimePoint& to) {
        if (from.get_type() != to.get_type())
            throw TimeDomainError("DomainTypes are incompatible");

        return DomainDuration(to.get_value() - from.get_value(), from.get_type());
    }


    // double arithmetic operators
    DomainDuration operator+(double other) const { return DomainDuration(m_value + other, m_type); }
    DomainDuration operator-(double other) const { return DomainDuration(m_value - other, m_type); }
    DomainDuration operator*(double factor) const { return DomainDuration(m_value * factor, m_type); }
    friend DomainDuration operator*(double factor, const DomainDuration& duration) { return duration * factor; }
    DomainDuration operator/(double denom) const { return DomainDuration(m_value / denom, m_type); }

    // TP arithmetic operators
    DomainTimePoint operator+(const TimePoint& other) const { return other + *this; }

    // DTP arithmetic operators
    /** throws TimeTypeError if `other` has a different type */
    DomainTimePoint operator+(const DomainTimePoint& other) const;

    // DD arithmetic operators
    /** throws TimeTypeError if `other` has a different type */
    DomainDuration operator+(const DomainDuration& other) const;
    /** throws TimeTypeError if `other` has a different type */
    DomainDuration operator-(const DomainDuration& other) const;
    /** throws TimeTypeError if `other` has a different type */
    DomainDuration operator*(const DomainDuration& other) const;
    /** throws TimeTypeError if `other` has a different type */
    DomainDuration operator/(const DomainDuration& other) const;


    bool operator==(const DomainDuration& other) const {
        return m_type == other.m_type && utils::equals(m_value, other.m_value);
    }


    bool operator!=(const DomainDuration& other) const { return !(*this == other); }


    DomainDuration as_type(DomainType type, const Meter& meter) const;


    std::string to_string(std::optional<std::size_t> num_decimals = std::nullopt) const {
        std::stringstream ss;

        ss << "DomainDuration(" << domain_type_to_string(m_type) << "=";
        if (num_decimals) {
            ss << std::fixed << std::setprecision(static_cast<int>(*num_decimals));
        }
        ss << m_value << ")";

        return ss.str();
    }


    std::string to_string_compact() const {
        return std::to_string(m_value) + " " + domain_type_to_string(m_type);
    }


    friend std::ostream& operator<<(std::ostream& os, const DomainDuration& obj) {
        os << obj.to_string();
        return os;
    }


    bool supports(const DomainTimePoint& t) const {
        return t.get_type() == m_type;
    }


    bool supports(const DomainDuration& other) const {
        return other.get_type() == m_type;
    }


    double get_value() const {
        return m_value;
    }


    DomainType get_type() const {
        return m_type;
    }

private:
    double m_value;
    DomainType m_type;
};


// ==============================================================================================

class DomainConverter {
public:
    DomainConverter() = delete;


    static double convert(double t, DomainType source_type, DomainType target_type, const Meter& meter) {
        if (source_type == target_type)
            return t;

        switch (source_type) {
            case DomainType::ticks:
                return target_type == DomainType::beats ? meter.ticks2beats(t) : meter.ticks2bars(t);
            case DomainType::beats:
                return target_type == DomainType::ticks ? meter.beats2ticks(t) : meter.beats2bars(t);
            case DomainType::bars:
                return target_type == DomainType::ticks ? meter.bars2ticks(t) : meter.bars2beats(t);
        }

        throw std::runtime_error("Unknown source type");
    }
};

// TODO: All code below should be non-inline and moved to a separate source file

// ==============================================================================================
// TimePoint
// ==============================================================================================


inline TimePoint TimePoint::operator+(double tick_increment) const {
    TimePoint t(*this);
    t.increment(tick_increment);
    return t;
}


inline TimePoint TimePoint::operator-(double tick_decrement) const {
    TimePoint t(*this);
    t.increment(-tick_decrement);
    return t;
}


inline TimePoint& TimePoint::operator+=(double tick_increment) {
    increment(tick_increment);
    return *this;
}


inline TimePoint& TimePoint::operator-=(double tick_decrement) {
    increment(-tick_decrement);
    return *this;
}


inline DomainTimePoint TimePoint::operator+(const DomainDuration& duration) const {
    return DomainTimePoint{get(duration.get_type()) + duration.get_value(), duration.get_type()};
}


inline DomainTimePoint TimePoint::operator-(const DomainDuration& duration) const {
    return DomainTimePoint{get(duration.get_type()) - duration.get_value(), duration.get_type()};
}


inline TimePoint& TimePoint::operator+=(const DomainDuration& duration) {
    increment(duration);
    return *this;
}


inline bool TimePoint::operator<(const DomainTimePoint& other) const { return other > *this; }
inline bool TimePoint::operator<=(const DomainTimePoint& other) const { return other >= *this; }
inline bool TimePoint::operator>(const DomainTimePoint& other) const { return other < *this; }
inline bool TimePoint::operator>=(const DomainTimePoint& other) const { return other <= *this; }


inline void TimePoint::increment(int64_t delta_nanos) {
    increment(nanos2ticks(delta_nanos));
}


inline void TimePoint::increment(double tick_increment) {
    m_tick += tick_increment;
    auto beat_increment = m_meter.ticks2beats(tick_increment);
    m_absolute_beat += beat_increment;
    m_relative_beat = utils::modulo(m_relative_beat + beat_increment, static_cast<double>(m_meter.get_numerator()));
    m_bar += m_meter.ticks2bars(tick_increment);
}


inline void TimePoint::increment(const DomainDuration& delta) {
    auto tick_increment = delta.as_type(DomainType::ticks, m_meter).get_value();
    increment(tick_increment);
}


inline bool TimePoint::increment_with_meter_change(int64_t delta_nanos, const Meter& new_meter) {
    return increment_with_meter_change(nanos2ticks(delta_nanos), new_meter);
}



inline bool TimePoint::increment_with_meter_change(double tick_increment, const Meter& new_meter) {
    // Case 1: Function called exactly at the start of a new bar - change meter before incrementing
    if (is_at_barline()) {
        m_meter = new_meter;
        increment(tick_increment);
        return true;
    }

    double ticks_in_current_meter = ticks_to_next_bar();

    // Case 2: Tick increment doesn't reach the next bar line - no meter change
    if (tick_increment < ticks_in_current_meter) {
        increment(tick_increment);
        return false;
    }

    // Case 3: Tick increment reaches or goes past the next bar line
    increment(ticks_in_current_meter);
    m_meter = new_meter;

    if (auto diff = tick_increment - ticks_in_current_meter; diff > 0) {
        increment(diff);
    }
    return true;
}


inline bool TimePoint::increment_with_meter_change(const DomainDuration& delta, const Meter& new_meter) {
    auto tick_increment = delta.as_type(DomainType::ticks, m_meter).get_value();
    return increment_with_meter_change(tick_increment, new_meter);
}




inline TimePoint TimePoint::incremented(double tick_increment) const {
    TimePoint t(*this);
    t.increment(tick_increment);
    return t;
}


inline TimePoint TimePoint::incremented(const DomainDuration& duration) const {
    TimePoint t(*this);
    t += duration;
    return t;
}



inline std::pair<TimePoint, bool> TimePoint::incremented_with_meter_change(double tick_increment
                                                                           , const Meter& new_meter) const {
    TimePoint t(*this);
    bool success = t.increment_with_meter_change(tick_increment, new_meter);
    return {t, success};
}


inline double TimePoint::distance_to(double time_value, DomainType type) const {
    switch (type) {
        case DomainType::ticks:
            return time_value - m_tick;
        case DomainType::beats:
            return time_value - m_absolute_beat;
        case DomainType::bars:
            return time_value - m_bar;
    }
    throw std::runtime_error("unsupported domain type");
}


// ==============================================================================================
// DomainTimePoint
// ==============================================================================================


inline DomainTimePoint DomainTimePoint::from_domain_duration(const DomainDuration& duration) {
    return {duration.get_value(), duration.get_type()};
}


inline DomainDuration DomainTimePoint::operator-(const TimePoint& other) const {
    return DomainDuration::distance(other, *this);
}


inline DomainTimePoint DomainTimePoint::operator+(const DomainDuration& duration) const {
    return duration + *this;
}


inline DomainDuration DomainTimePoint::operator-(const DomainTimePoint& other) const {
    return DomainDuration::distance(other, *this);
}


inline DomainTimePoint DomainTimePoint::operator-(const DomainDuration& other) const {
    if (!supports(other))
        throw TimeDomainError("DomainTypes are incompatible");

    return {m_value - other.get_value(), m_type};
}


inline bool DomainTimePoint::supports(const DomainDuration& other) const { return other.get_type() == m_type; }


inline DomainTimePoint DomainTimePoint::as_type(DomainType target_type, const TimePoint& last_transport_tp) const {
    if (m_type == target_type) {
        return *this;
    }

    auto delta_source = last_transport_tp.distance_to(m_value, m_type);
    auto delta_target = DomainConverter::convert(delta_source, m_type, target_type, last_transport_tp.get_meter());
    auto time_in_target_type = last_transport_tp.get(target_type) + delta_target;

    return {time_in_target_type, target_type};
}


// ==============================================================================================
// DomainDuration
// ==============================================================================================


inline DomainTimePoint DomainDuration::operator+(const DomainTimePoint& other) const {
    if (!supports(other))
        throw TimeDomainError("DomainTypes are incompatible");

    return DomainTimePoint{m_value + other.get_value(), m_type};
}


inline DomainDuration DomainDuration::operator+(const DomainDuration& other) const {
    if (!supports(other)) {
        throw TimeDomainError("DomainTypes are incompatible");
    }
    return DomainDuration(m_value + other.m_value, m_type);
}


inline DomainDuration DomainDuration::operator-(const DomainDuration& other) const {
    if (!supports(other)) {
        throw TimeDomainError("DomainTypes are incompatible");
    }
    return DomainDuration(m_value - other.m_value, m_type);
}


inline DomainDuration DomainDuration::operator*(const DomainDuration& other) const {
    if (!supports(other))
        throw TimeDomainError("DomainTypes are incompatible");

    return DomainDuration(m_value * other.m_value, m_type);
}


inline DomainDuration DomainDuration::operator/(const DomainDuration& other) const {
    if (!supports(other))
        throw TimeDomainError("DomainTypes are incompatible");

    return DomainDuration(m_value / other.m_value, m_type);
}


inline DomainDuration DomainDuration::as_type(DomainType target_type, const Meter& meter) const {
    if (m_type == target_type) {
        return *this;
    }

    return DomainDuration{DomainConverter::convert(m_value, m_type, target_type, meter), m_type};
}
} // namespace serialist

#endif //SERIALISTLOOPER_TIME_POINT_H
