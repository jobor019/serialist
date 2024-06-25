
#ifndef SERIALISTLOOPER_TIME_POINT_GENERATORS_H
#define SERIALISTLOOPER_TIME_POINT_GENERATORS_H

#include "time_point.h"

class FreePeriodic {
public:

};


class TransportLocked {
public:
    static constexpr double EPSILON = 1e-8;

    TransportLocked() = delete;

    static DomainTimePoint next(const TimePoint& current_time
                                , const DomainDuration& period
                                , const DomainDuration& offset
                                , bool is_first_value
                                , bool enforce_period_as_minimum_duration = false
                                , double epsilon = EPSILON) {
        auto type = period.get_type();
        auto t = current_time.get(type);
        auto p = period.get_value();
        auto o = offset.as_type(type, current_time.get_meter()).get_value();

        if (enforce_period_as_minimum_duration) {
            return {compute_next(t + p, p, o, true, epsilon), type};
        } else {
            return {compute_next(t, p, o, is_first_value, epsilon), type};
        }

    }


    /** @throws TimeDomainError if current_time is not compatible with period */
    static DomainTimePoint next(const DomainTimePoint& current_time
                                , const DomainDuration& period
                                , const DomainDuration& offset
                                , const Meter& current_meter
                                , bool is_first_value = false
                                , bool enforce_period_as_minimum_duration = false
                                , double epsilon = EPSILON) {
        if (!period.supports(current_time)) {
            throw TimeDomainError("DomainTypes are incompatible");
        }
        auto type = period.get_type();
        auto t = current_time.get_value();
        auto p = period.get_value();
        auto o = offset.as_type(type, current_meter).get_value();

        if (enforce_period_as_minimum_duration) {
            return {compute_next(t + p, p, o, true, epsilon), type};
        } else {
            return {compute_next(t, p, o, is_first_value, epsilon), type};
        }
    }


    /** @throws TimeDomainError if t1 is not compatible with period */
    static DomainTimePoint next_from_either(const DomainTimePoint& t1
                                            , const TimePoint& current_time
                                            , const DomainDuration& period
                                            , const DomainDuration& offset
                                            , bool is_first_value = false
                                            , bool enforce_period_as_minimum_duration = false
                                            , double epsilon = EPSILON) {
        if (!period.supports(t1)) {
            throw TimeDomainError("DomainTypes are incompatible");
        }

        auto next_trigger = next(t1, period, offset, current_time.get_meter()
                                 , is_first_value, enforce_period_as_minimum_duration, epsilon);

        if (!next_trigger.elapsed(current_time)) {
            // next trigger is in the future: return it
            return next_trigger;
        } else if (next_trigger - epsilon <= current_time) {
            // next trigger is less than or equal to epsilon in the past:
            // return the next value on the grid after current_time using is_first_value
            return next(current_time, period, offset, is_first_value, false, epsilon);
        }

        // next trigger is more than epsilon in the past: return the next value on the grid (using is_first_value=true)
        return next(current_time, period, offset, true, false, epsilon);
    }


    /** @throws TimeDomainError if target is not compatible with period */
    static DomainTimePoint adjusted(const DomainTimePoint& target
                                    , const TimePoint& current_time
                                    , const DomainDuration& period
                                    , const DomainDuration& offset
                                    , double epsilon = EPSILON) {
        if (!period.supports(target)) {
            throw TimeDomainError("DomainTypes are incompatible");
        }

        auto phase = phase_of(target, period, offset, current_time.get_meter());
        DomainTimePoint adjusted_target = target + period * std::round(phase);

        if (adjusted_target < current_time) {
            return next(current_time, period, offset, true, false, epsilon);
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

    /**
     * Finds the next point in time on the grid specified by period and offset
     *
     * @param current_time   the point in time to start from
     * @param period         the period of the grid
     * @param offset         the offset of the grid
     * @param is_first_value if true, will return current_time if current_time is on the grid ± epsilon
     *                       (may in other words return values slightly before or after current_time)
     * @param epsilon        the tolerance for the algorithm. It will not return a value that is less than epsilon
     *                       ahead of current_time unless is_first_value is true
     */
    static double compute_next(double current_time
                               , double period
                               , double offset
                               , bool is_first_value = false
                               , double epsilon = EPSILON) {
        auto rem = utils::modulo(current_time - offset, period);

        if (is_first_value && utils::equals(rem, 0.0, epsilon)) {
            // current time is slightly ahead of the grid but first_value is true:
            // return the point in time right before current_time that is on the grid
            return current_time - rem;
        } else if (!is_first_value && period - rem < epsilon) {
            // current time is less than epsilon before the next point on the grid:
            // return the point after the next on the grid
            return current_time - rem + 2 * period;
        } else {
            // current time is either greater than or equal to epsilon before the next point on the grid,
            // or less than but is_first_value is true: return the next point on the grid
            return current_time - rem + period;
        }
    }

    static double compute_phase(double t, double period, double offset) {
        return utils::modulo(t - offset, period) / period;
    }

};

#endif //SERIALISTLOOPER_TIME_POINT_GENERATORS_H
