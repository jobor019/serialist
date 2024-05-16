
#ifndef SERIALISTLOOPER_TIME_SPECIFICATION_H
#define SERIALISTLOOPER_TIME_SPECIFICATION_H

#include "core/algo/time/time_point.h"
#include "core/collections/vec.h"
#include "core/algo/facet.h"
#include "core/exceptions.h"


class TimeSpecification {
public:
    TimeSpecification() = default;

    virtual ~TimeSpecification() = default;

    TimeSpecification(const TimeSpecification&) = default;

    TimeSpecification& operator=(const TimeSpecification&) = default;

    TimeSpecification(TimeSpecification&&) noexcept = default;

    TimeSpecification& operator=(TimeSpecification&&) noexcept = default;

    virtual std::unique_ptr<TimeSpecification> clone() const = 0;

    virtual bool supports(const DomainTimePoint& t) const = 0;

//    virtual std::unique_ptr<TimeSpecification> as_type(DomainType type) const = 0;

    /** throws ParameterError if t is not supported by this specification */
    virtual DomainTimePoint next(const DomainTimePoint& last_trigger_time, const Meter& current_meter) const = 0;

    /** throws ParameterError if t is not supported by this specification */
    DomainTimePoint next(const DomainTimePoint& last_trigger_time, const TimePoint& current_time) const {
        return next(last_trigger_time, current_time.get_meter());
    }

    virtual DomainTimePoint next(const TimePoint& current_time, bool is_first_value) const noexcept = 0;


    virtual DomainType get_type() const = 0;
};


// ==============================================================================================

class Period : public TimeSpecification {
public:
    explicit Period(double value = 1.0, DomainType type = DomainType::ticks)
            : m_duration(utils::clip(value, {0.0}), type) {}

    explicit Period(const DomainDuration& duration) : m_duration(duration) {}

    std::unique_ptr<TimeSpecification> clone() const override {
        return std::make_unique<Period>(m_duration);
    }

    bool supports(const DomainTimePoint& t) const override {
        return t.get_type() == m_duration.get_type();
    }

//    std::unique_ptr<TimeSpecification> as_type(DomainType type) const override {
//        return std::make_unique<SingleDomainDuration>(m_value, type);
//    }

    DomainTimePoint next(const TimePoint& current_time, bool is_first_value) const noexcept override {
        if (is_first_value) {
            return DomainTimePoint::from_time_point(current_time, get_type());
        }

        return current_time + m_duration;
    }

    /** throws TimeTypeError if `other` has a different type */
    DomainTimePoint next(const DomainTimePoint& last_trigger_time, const Meter&) const override {
        return last_trigger_time + m_duration;
    }

    DomainType get_type() const override {
        return m_duration.get_type();
    }

    const DomainDuration& get_duration() const {
        return m_duration;
    }

private:
    DomainDuration m_duration;

};


// ==============================================================================================


// TODO: Not sure if this class will ever be needed
//class MultiDomainPeriod : public TimeSpecification {
//public:
//
//private:
//    Vec<DomainDuration> m_values;
//};


// ==============================================================================================

class GridPosition : public TimeSpecification {
public:
    GridPosition(const DomainDuration& period
                 , const DomainDuration& offset)
            : m_period(period)
              , m_offset(offset) {
    }

    std::unique_ptr<TimeSpecification> clone() const override {
        return std::make_unique<GridPosition>(m_period, m_offset);
    }

    bool supports(const DomainTimePoint& t) const override {
        return t.get_type() == m_period.get_type();
    }

//    std::unique_ptr<TimeSpecification> as_type(DomainType type) const override {}

    DomainTimePoint next(const TimePoint& current_time, bool is_first_value) const noexcept override {
        auto type = get_type();
        auto t = current_time.get(type);
        auto period = m_period.get_value();
        auto offset = m_offset.as_type(type, current_time.get_meter()).get_value();

        return {compute_next(t, period, offset, is_first_value), get_type()};
    }

    DomainTimePoint next(const DomainTimePoint& last_trigger_time, const Meter& current_meter) const override {
        if (!supports(last_trigger_time)) {
            throw TimeDomainError("DomainTypes are incompatible");
        }
        auto type = get_type();
        auto t = last_trigger_time.get_value();
        auto period = m_period.get_value();
        auto offset = m_offset.as_type(type, current_meter).get_value();

        return {compute_next(t, period, offset), get_type()};

    }

    DomainType get_type() const override {
        return m_period.get_type();
    }

private:
    static double compute_next(double current_time, double period, double offset, bool is_first_value = false) {
        auto rem = utils::modulo(current_time, period);
        if (rem < offset || (is_first_value && utils::equals(rem, offset, 1e-8))) {
            return current_time + offset - rem;
        }
        return current_time + offset - rem + period;
    }


    DomainDuration m_period;
    DomainDuration m_offset;
};


// ==============================================================================================

namespace utils {

constexpr inline double NO_OFFSET = -1.0;

// ==============================================================================================

inline std::unique_ptr<TimeSpecification> ts_from_duration_offset(const DomainDuration& duration
                                                                  , const std::optional<DomainDuration>& offset) {
    if (offset) {
        return std::make_unique<GridPosition>(duration, *offset);
    }

    return std::make_unique<Period>(duration);
}

inline std::unique_ptr<TimeSpecification> ts_from_duration_offset(double duration_value
                                                                  , DomainType duration_type
                                                                  , double offset_value
                                                                  , std::optional<DomainType> offset_type) {
    auto duration = DomainDuration(duration_value, duration_type);
    auto offset = offset_type ? std::make_optional<DomainDuration>(offset_value, *offset_type) : std::nullopt;
    return ts_from_duration_offset(duration, offset);
}

inline Vec <std::unique_ptr<TimeSpecification>> ts_from_durations_offsets(const Vec<double>& duration_values
                                                                          , DomainType duration_type
                                                                          , const Vec<double>& offset_values
                                                                          , std::optional<DomainType> offset_type) {
    assert(duration_values.size() == offset_values.size());

    Vec <std::unique_ptr<TimeSpecification>> time_specs;
    for (std::size_t i = 0; i < duration_values.size(); ++i) {
        time_specs.append(ts_from_duration_offset(duration_values[i], duration_type, offset_values[i], offset_type));
    }

    return time_specs;
}

// ==============================================================================================


inline std::optional<DomainType> parse_offset_type(const Facet& offset_type) {
    if (offset_type >= 0.0) {
        return static_cast<DomainType>(offset_type);
    }
    return std::nullopt;
}

inline std::optional<DomainType> parse_offset_type(const std::optional<Facet>& offset_type) {
    if (offset_type) {
        return parse_offset_type(*offset_type);
    }
    return std::nullopt;
}

} // utils

#endif //SERIALISTLOOPER_TIME_SPECIFICATION_H
