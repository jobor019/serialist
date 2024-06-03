
#ifndef SERIALISTLOOPER_TIME_POINT_H
#define SERIALISTLOOPER_TIME_POINT_H

#include "meter.h"
#include "core/utility/math.h"
#include "core/algo/fraction.h"
#include "core/exceptions.h"
#include <cmath>
#include <chrono>
#include <string>

enum class DomainType {
    ticks, beats, bars
};


// ==============================================================================================

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
              , m_transport_running(transport_running) {}

    static TimePoint zero() { return TimePoint(); }

    TimePoint operator+(double tick_increment) {
        TimePoint t(*this);
        t.increment(tick_increment);
        return t;
    }

    TimePoint& operator+=(double tick_increment) {
        increment(tick_increment);
        return *this;
    }

    TimePoint& operator+=(const DomainDuration& duration);

    TimePoint operator-(double tick_decrement) {
        TimePoint t(*this);
        t.increment(-tick_decrement);
        return t;
    }

    TimePoint operator-=(double tick_decrement) {
        increment(-tick_decrement);
        return *this;
    }


    void increment(int64_t delta_nanos) {
        increment(static_cast<double>(delta_nanos) * 1e-9 * m_tempo / 60.0);
    }

    void increment(double tick_increment) {
        m_tick += tick_increment;
        auto beat_increment = m_meter.ticks2beats(tick_increment);
        m_absolute_beat += beat_increment;
        m_relative_beat = utils::modulo(m_relative_beat + beat_increment, m_meter.duration());
        m_bar += m_meter.ticks2bars(tick_increment);
    }

    TimePoint incremented(double tick_increment) {
        TimePoint t(*this);
        t.increment(tick_increment);
        return t;
    }

    double distance_to(double time_value, DomainType type) const {
        switch (type) {
            case DomainType::ticks:
                return time_value - m_tick;
            case DomainType::beats:
                return time_value - m_absolute_beat;
            case DomainType::bars:
                return time_value - m_bar;
        }
    }


    double next_tick_of(const Fraction& quantization_level = {1, 4}) const {
        (void) quantization_level;
        throw std::runtime_error("not implemented"); // TODO: Probably not the right place to implement quantization
//        auto q = static_cast<double>(quantization_level);
//        auto diff = fmod(m_beat, q);
//
//        if (diff < 1e-4)
//            return m_tick - diff; // schedule up to 0.0001 ticks in the past
//
//        return m_tick - diff + q;   // schedule on next quantization level
    }

    bool operator<(const TimePoint& other) const {
        return m_tick < other.m_tick;
    }

    bool operator<=(const TimePoint& other) const {
        return m_tick <= other.m_tick;
    }

    bool operator>(const TimePoint& other) const {
        return m_tick > other.m_tick;
    }

    bool operator>=(const TimePoint& other) const {
        return m_tick >= other.m_tick;
    }

    explicit operator std::string() const {
        return to_string();
    }


    double get_tick() const { return m_tick; }

    double get_tempo() const { return m_tempo; }

    double get_relative_beat() const { return m_relative_beat; }

    double get_absolute_beat() const { return m_absolute_beat; }

    double get_bar() const { return m_bar; }

    double get(DomainType type) const {
        switch (type) {
            case DomainType::ticks:
                return m_tick;
            case DomainType::beats:
                return m_absolute_beat;
            case DomainType::bars:
                return m_bar;
        }
    }

    const Meter& get_meter() const { return m_meter; }

    bool get_transport_running() const { return m_transport_running; }

    std::string to_string() const {
        return "TimePoint("
               "tick=" + std::to_string(m_tick)
               + ", beat=" + std::to_string(m_absolute_beat)
               + ", bar=" + std::to_string(m_bar)
               + ", tempo=" + std::to_string(m_tempo)
               + ", meter=" + m_meter.to_string()
               + ", transport_running=" + (m_transport_running ? "true" : "false")
               + ")";
    }


private:
    double m_tick;
    double m_tempo;
    double m_absolute_beat;
    double m_relative_beat;
    double m_bar;
    Meter m_meter;
    bool m_transport_running;


};


// ==============================================================================================


class DomainTimePoint {
public:
    DomainTimePoint(double value, DomainType type) : m_value(value), m_type(type) {}

    static DomainTimePoint from_time_point(const TimePoint& t, DomainType type) { return {t.get(type), type}; }

    static DomainTimePoint zero() { return {0.0, DomainType::ticks}; }

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

    DomainDuration operator-(const DomainTimePoint& other) const;

    bool operator<(const TimePoint& t) const { return m_value < t.get(m_type); }
    bool operator<=(const TimePoint& t) const { return m_value <= t.get(m_type); }
    bool operator>(const TimePoint& t) const { return m_value > t.get(m_type); }
    bool operator>=(const TimePoint& t) const { return m_value >= t.get(m_type); }

    friend bool operator<(const TimePoint& t, const DomainTimePoint& other) { return other > t; }
    friend bool operator<=(const TimePoint& t, const DomainTimePoint& other) { return other >= t; }
    friend bool operator>(const TimePoint& t, const DomainTimePoint& other) { return other < t; }
    friend bool operator>=(const TimePoint& t, const DomainTimePoint& other) { return other <= t; }

    DomainTimePoint operator+(double other) const { return {m_value + other, m_type}; }
    DomainTimePoint operator-(double other) const { return {m_value - other, m_type}; }
    DomainTimePoint operator*(double other) const { return {m_value * other, m_type}; }

    DomainTimePoint operator+(const TimePoint& other) const { return {m_value + other.get(m_type), m_type}; }
    DomainTimePoint operator-(const TimePoint& other) const { return {m_value - other.get(m_type), m_type}; }

    friend DomainTimePoint operator+(const TimePoint& other, const DomainTimePoint& t) { return t + other; }

    friend DomainTimePoint operator-(const TimePoint& other, const DomainTimePoint& t) {
        return DomainTimePoint{other.get(t.m_type) - t.m_value, t.m_type};
    }

    DomainTimePoint& operator+=(double other) {
        m_value += other;
        return *this;
    }

    DomainTimePoint& operator-=(double other) {
        m_value -= other;
        return *this;
    }


    // TODO: Not sure if this will be needed. If so, should be out of line
//    DomainDuration operator-(const TimePoint& other) const;
//    DomainDuration operator-(const DomainDuration& other) const;

    DomainTimePoint as_type(DomainType target_type, const TimePoint& last_transport_tp) const;


    bool elapsed(const TimePoint& current_time) const {
        return current_time.get(m_type) >= m_value;
    }

    DomainType get_type() const { return m_type; }

    double get_value() const { return m_value; }

private:
    double m_value;
    DomainType m_type;
};


// ==============================================================================================

class DomainDuration {
public:
    explicit DomainDuration(double value = 1.0, DomainType type = DomainType::ticks)
            : m_value(value), m_type(type) {}

    /** throws TimeTypeError if `other` has a different type */
    static DomainDuration distance(const DomainTimePoint& from, const DomainTimePoint& to) {
        if (from.get_type() != to.get_type())
            throw TimeDomainError("DomainTypes are incompatible");

        return DomainDuration(to.get_value() - from.get_value(), from.get_type());
    }

    DomainDuration as_type(DomainType type, const Meter& meter) const;


    DomainTimePoint operator+(const TimePoint& other) const {
        return {other.get(m_type) + m_value, m_type};
    }

    /** throws TimeTypeError if `other` has a different type */
    DomainTimePoint operator+(const DomainTimePoint& other) const {
        if (!supports(other))
            throw TimeDomainError("DomainTypes are incompatible");

        return DomainTimePoint{m_value + other.get_value(), m_type};
    }

    friend DomainTimePoint operator+(const TimePoint& lhs, const DomainDuration& rhs) {
        return rhs + lhs;
    }

    /** throws TimeTypeError if `other` has a different type */
    friend DomainTimePoint operator+(const DomainTimePoint& lhs, const DomainDuration& rhs) {
        return rhs + lhs;
    }

    /** throws TimeTypeError if `other` has a different type */
    DomainTimePoint operator-(const DomainTimePoint& other) const {
        if (!supports(other))
            throw TimeDomainError("DomainTypes are incompatible");

        return {m_value - other.get_value(), m_type};
    }

    /** throws TimeTypeError if `other` has a different type */
    friend DomainTimePoint operator-(const DomainTimePoint& lhs, const DomainDuration& rhs) {
        if (!rhs.supports(lhs))
            throw TimeDomainError("DomainTypes are incompatible");

        return {lhs.get_value() - rhs.m_value, lhs.get_type()};
    }

    bool operator==(const DomainDuration& other) const {
        return m_type == other.m_type && utils::equals(m_value, other.m_value);
    }

    bool operator!=(const DomainDuration& other) const {
        return !(*this == other);
    }

    // TODO: Not a valid function
//    /** throws TimeTypeError if `other` has a different type */
//    friend DomainDuration operator-(const DomainTimePoint& lhs, const DomainTimePoint& rhs) {
//        if (lhs.get_type() != rhs.get_type())
//            throw TimeDomainError("DomainTypes are incompatible");
//
//        return DomainDuration(lhs.get_value() - rhs.get_value(), lhs.get_type());
//    }



    DomainDuration operator*(double factor) const {
        return DomainDuration(m_value * factor, m_type);
    }

    friend DomainDuration operator*(double factor, const DomainDuration& duration) {
        return duration * factor;
    }

    bool supports(const DomainTimePoint& t) const {
        return t.get_type() == m_type;
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


/**
 * Placeholder for a class describing a directed period of time. That is, the interval between two DomainTimePoints
 * (or equivalently, the positive or negative interval described by a DomainTimePoint and a DomainDuration).
 */
class DomainInterval {
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
                return (target_type == DomainType::beats) ? meter.ticks2beats(t) : meter.ticks2bars(t);
            case DomainType::beats:
                return (target_type == DomainType::ticks) ? meter.beats2ticks(t) : meter.beats2bars(t);
            case DomainType::bars:
                return (target_type == DomainType::ticks) ? meter.bars2ticks(t) : meter.bars2beats(t);
        }
    }
};


// ==============================================================================================

inline TimePoint& TimePoint::operator+=(const DomainDuration& duration) {
    auto tick_increment = duration.as_type(DomainType::ticks, m_meter).get_value();
    increment(tick_increment);
    return *this;
}

// ==============================================================================================

inline DomainTimePoint DomainTimePoint::as_type(DomainType target_type, const TimePoint& last_transport_tp) const {
    if (m_type == target_type) {
        return *this;
    }

    auto delta_source = last_transport_tp.distance_to(m_value, m_type);
    auto delta_target = DomainConverter::convert(delta_source, m_type, target_type, last_transport_tp.get_meter());
    auto time_in_target_type = last_transport_tp.get(target_type) + delta_target;

    return {time_in_target_type, target_type};
}

inline DomainDuration DomainTimePoint::operator-(const DomainTimePoint& other) const {
    return DomainDuration::distance(other, *this);
}

// ==============================================================================================


inline DomainDuration DomainDuration::as_type(DomainType target_type, const Meter& meter) const {
    if (m_type == target_type) {
        return *this;
    }

    return DomainDuration{DomainConverter::convert(m_value, m_type, target_type, meter), m_type};
}


#endif //SERIALISTLOOPER_TIME_POINT_H
