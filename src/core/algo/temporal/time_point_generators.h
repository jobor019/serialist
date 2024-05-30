
#ifndef SERIALISTLOOPER_TIME_POINT_GENERATORS_H
#define SERIALISTLOOPER_TIME_POINT_GENERATORS_H

#include "time_point.h"


class TransportLocked {
public:
    static constexpr double EPSILON = 1e-8;

    TransportLocked() = delete;

    static DomainTimePoint next(const TimePoint& current_time
                                , const DomainDuration& period
                                , const DomainDuration& offset
                                , bool is_first_value
                                , double epsilon = EPSILON) {
        auto type = period.get_type();
        auto t = current_time.get(type);
        auto p = period.get_value();
        auto o = offset.as_type(type, current_time.get_meter()).get_value();

        return {compute_next(t, p, o, is_first_value, epsilon), type};
    }

    /** @throws TimeDomainError if last_trigger_time is not compatible with period */
    static DomainTimePoint next(const DomainTimePoint& last_trigger_time
                                , const DomainDuration& period
                                , const DomainDuration& offset
                                , const Meter& current_meter
                                , bool is_first_value = false
                                , double epsilon = EPSILON) {
        if (!period.supports(last_trigger_time)) {
            throw TimeDomainError("DomainTypes are incompatible");
        }
        auto type = period.get_type();
        auto t = last_trigger_time.get_value();
        auto p = period.get_value();
        auto o = offset.as_type(type, current_meter).get_value();

        return {compute_next(t, p, o, is_first_value, epsilon), type};
    }


    /**
     * @brief returns the next DomainTimePoint on the grid after last_trigger_time if it's greater than current_time,
     *        otherwise returns the next DomainTimePoint on the grid equal to or greater than current_time
     */
    static DomainTimePoint next_from_either(const DomainTimePoint& last_trigger_time
                                            , const TimePoint& current_time
                                            , const DomainDuration& period
                                            , const DomainDuration& offset
                                            , bool is_first_value = false
                                            , double epsilon = EPSILON) {
        if (period.supports(last_trigger_time)) {
            auto next_trigger = next(last_trigger_time, period, offset, current_time.get_meter(), is_first_value
                                     , epsilon);
            if (!next_trigger.elapsed(current_time)) {
                return next_trigger;
            }
        }

        return next(last_trigger_time, period, offset, current_time.get_meter(), true, epsilon);
    }


    /** @throws TimeDomainError if target is not compatible with period */
    static DomainTimePoint adjusted(const DomainTimePoint& target
                                    , const TimePoint& current_time
                                    , const DomainDuration& period
                                    , const DomainDuration& offset) {
        if (!period.supports(target)) {
            throw TimeDomainError("DomainTypes are incompatible");
        }

        auto phase = phase_of(target, period, offset, current_time.get_meter());
        DomainTimePoint adjusted_target = target + period * std::round(phase);

        if (adjusted_target < current_time) {
            return next(current_time, period, offset, true);
        } else {
            return adjusted_target;
        }
    }


    static double phase_of(const TimePoint& t, const DomainDuration& period, const DomainDuration& offset) {
        auto type = period.get_type();
        auto p = period.get_value();
        auto o = offset.as_type(type, t.get_meter()).get_value();
        return compute_phase(t.get(type), p, o);
    }


    /** @throws TimeDomainError if t is not compatible with period */
    static double phase_of(const DomainTimePoint& dtp
                           , const DomainDuration& period
                           , const DomainDuration& offset
                           , const Meter& current_meter) {
        if (!period.supports(dtp)) {
            throw TimeDomainError("DomainTypes are incompatible");
        }

        auto type = period.get_type();
        auto p = period.get_value();
        auto o = offset.as_type(type, current_meter).get_value();
        return compute_phase(dtp.get_value(), p, o);
    }


private:
    static double compute_next(double current_time
                               , double period
                               , double offset
                               , bool is_first_value = false
                               , double epsilon = EPSILON) {
        auto rem = utils::modulo(current_time - offset, period);

        if (is_first_value && utils::equals(rem, 0.0, epsilon)) {
            return current_time - rem;
        } else if (period - rem < epsilon) {
            return current_time - rem + 2 * period;
        } else {
            return current_time - rem + period;
        }
    }

    static double compute_phase(double t, double period, double offset) {
        return utils::modulo(t - offset, period) / period;
    }

};

#endif //SERIALISTLOOPER_TIME_POINT_GENERATORS_H
