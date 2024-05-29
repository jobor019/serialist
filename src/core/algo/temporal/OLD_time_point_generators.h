
#ifndef SERIALISTLOOPER_OLD_TIME_POINT_GENERATORS_H
#define SERIALISTLOOPER_OLD_TIME_POINT_GENERATORS_H

#include "core/algo/temporal/time_point.h"
#include "core/collections/vec.h"
#include "core/algo/facet.h"
#include "core/exceptions.h"


class TimePointGenerator {
public:
    TimePointGenerator() = default;

    virtual ~TimePointGenerator() = default;

    TimePointGenerator(const TimePointGenerator&) = default;

    TimePointGenerator& operator=(const TimePointGenerator&) = default;

    TimePointGenerator(TimePointGenerator&&) noexcept = default;

    TimePointGenerator& operator=(TimePointGenerator&&) noexcept = default;

    virtual std::unique_ptr<TimePointGenerator> clone() const = 0;

    virtual bool supports(const DomainTimePoint& t) const = 0;

    /** throws ParameterError if t is not supported by this specification */
    virtual DomainTimePoint next(const DomainTimePoint& last_trigger_time, const Meter& current_meter) const = 0;


    virtual DomainTimePoint next(const TimePoint& current_time, bool is_first_value) const noexcept = 0;

    /** throws ParameterError if t is not supported by this specification */
    DomainTimePoint next(const DomainTimePoint& last_trigger_time, const TimePoint& current_time) const {
        return next(last_trigger_time, current_time.get_meter());
    }


    virtual DomainType get_type() const = 0;
};


// ==============================================================================================

/**
 * Generates a time point at regular intervals without a fixed relation to transport.
 * $$t_{i+1} = t_i + d$$
 */
class FreePeriodicTimePoint : public TimePointGenerator {
public:
    explicit FreePeriodicTimePoint(double value = 1.0, DomainType type = DomainType::ticks)
            : m_duration(utils::clip(value, {0.0}), type) {}

    explicit FreePeriodicTimePoint(const DomainDuration& duration) : m_duration(duration) {}

    std::unique_ptr<TimePointGenerator> clone() const override {
        return std::make_unique<FreePeriodicTimePoint>(m_duration);
    }

    bool supports(const DomainTimePoint& t) const override {
        return t.get_type() == m_duration.get_type();
    }


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

/**
 * Generates a time point at regular intervals based on absolute position in transport / meter.
 */
//class TransportLockedTimePoint : public TimePointGenerator {
//public:
//    static constexpr double EPSILON = 1e-8;
//
//    TransportLockedTimePoint(const DomainDuration& period
//                             , const DomainDuration& offset)
//            : m_period(period)
//              , m_offset(offset) {
//    }
//
//    std::unique_ptr<TimePointGenerator> clone() const override {
//        return std::make_unique<TransportLockedTimePoint>(m_period, m_offset);
//    }
//
//    bool supports(const DomainTimePoint& t) const override {
//        return t.get_type() == m_period.get_type();
//    }
//
//    DomainTimePoint next(const TimePoint& current_time, bool is_first_value) const noexcept override {
//        auto type = get_type();
//        auto t = current_time.get(type);
//        auto period = m_period.get_value();
//        auto offset = m_offset.as_type(type, current_time.get_meter()).get_value();
//
//        return {compute_next(t, period, offset, is_first_value), get_type()};
//    }
//
//    DomainTimePoint next(const DomainTimePoint& last_trigger_time, const Meter& current_meter) const override {
//        if (!supports(last_trigger_time)) {
//            throw TimeDomainError("DomainTypes are incompatible");
//        }
//        auto type = get_type();
//        auto t = last_trigger_time.get_value();
//        auto period = m_period.get_value();
//        auto offset = m_offset.as_type(type, current_meter).get_value();
//
//        return {compute_next(t, period, offset), get_type()};
//    }
//
//    double phase_of(const TimePoint& t) const {
//        auto type = get_type();
//        auto period = m_period.get_value();
//        auto offset = m_offset.as_type(type, t.get_meter()).get_value();
//        return utils::modulo(t.get(type) - offset, period) / period;
//    }
//
//    DomainType get_type() const override {
//        return m_period.get_type();
//    }
//
//private:
//    static double compute_next(double current_time, double period, double offset, bool is_first_value = false) {
//        auto rem = utils::modulo(current_time - offset, period);
//
//        if (is_first_value && utils::equals(rem, 0.0, EPSILON)) {
//            return current_time - rem;
//        } else if (period - rem < EPSILON) {
//            return current_time - rem + 2 * period;
//        } else {
//            return current_time - rem + period;
//        }
//    }
//
//
//    DomainDuration m_period;
//    DomainDuration m_offset;
//};


// ==============================================================================================
//
//namespace temporal {
//
//inline std::unique_ptr<TimePointGenerator> ts_from_duration_offset(const DomainDuration& duration
//                                                                   , const DomainDuration& offset
//                                                                   , bool offset_enabled) {
//    if (offset_enabled)
//        return std::make_unique<TransportLockedTimePoint>(duration, offset);
//
//    return std::make_unique<FreePeriodicTimePoint>(duration);
//}
//
//inline std::unique_ptr<TimePointGenerator> ts_from_duration_offset(double duration_value
//                                                                   , DomainType duration_type
//                                                                   , double offset_value
//                                                                   , DomainType offset_type
//                                                                   , bool offset_enabled) {
//    auto duration = DomainDuration(duration_value, duration_type);
//    auto offset = DomainDuration(offset_value, offset_type);
//    return ts_from_duration_offset(duration, offset, offset_enabled);
//}
//
//inline Vec<std::unique_ptr<TimePointGenerator>> ts_from_durations_offsets(const Vec<double>& duration_values
//                                                                          , DomainType duration_type
//                                                                          , const Vec<double>& offset_values
//                                                                          , DomainType offset_type
//                                                                          , bool offset_enabled) {
//    assert(duration_values.size() == offset_values.size());
//
//    Vec<std::unique_ptr<TimePointGenerator>> time_specs;
//    for (std::size_t i = 0; i < duration_values.size(); ++i) {
//        time_specs.append(ts_from_duration_offset(duration_values[i]
//                                                  , duration_type
//                                                  , offset_values[i]
//                                                  , offset_type
//                                                  , offset_enabled));
//    }
//
//    return time_specs;
//}

} // temporal

#endif //SERIALISTLOOPER_OLD_TIME_POINT_GENERATORS_H
