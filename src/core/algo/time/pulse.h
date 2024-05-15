
#ifndef SERIALISTLOOPER_PULSE_H
#define SERIALISTLOOPER_PULSE_H

#include <optional>
#include "core/algo/time/trigger.h"
#include "core/collections/vec.h"
#include "core/collections/held.h"
#include "time_specification.h"
#include "core/exceptions.h"

class Pulse {
public:
    Pulse(std::size_t id, const DomainTimePoint& trigger_time
          , const std::optional<DomainTimePoint>& pulse_off_time = std::nullopt)
            : m_id(id)
              , m_trigger_time(trigger_time)
              , m_pulse_off_time(pulse_off_time) {
    }

    bool elapsed(const TimePoint& current_time) const {
        return m_pulse_off_time && m_pulse_off_time->elapsed(current_time);
    }

    bool has_pulse_off() const {
        return static_cast<bool>(m_pulse_off_time);
    }

    std::size_t get_id() const { return m_id; }

    void set_pulse_off(const DomainTimePoint& time) {
        m_pulse_off_time = time;
    }

//    void set_type(DomainType type, const TimePoint& last_transport_time) {
//        if (m_pulse_off_time) {
//            m_pulse_off_time = m_pulse_off_time->as_type(type, last_transport_time);
//        }
//
//        m_trigger_time = m_trigger_time.as_type(type, last_transport_time);
//    }

    DomainTimePoint get_trigger_time() const { return m_trigger_time; }

    const std::optional<DomainTimePoint>& get_pulse_off_time() const { return m_pulse_off_time; }

private:
    std::size_t m_id;
    DomainTimePoint m_trigger_time;
    std::optional<DomainTimePoint> m_pulse_off_time;
};


// ==============================================================================================


class Pulses : public Flushable<Trigger> {
public:

    std::optional<Trigger> new_pulse(const DomainTimePoint& pulse_on_time
                      , const std::optional<DomainTimePoint>& pulse_off_time
                      , std::optional<std::size_t> id = std::nullopt) {
//        if (pulse_on_time.get_type() != m_type) {
//            throw ParameterError("pulse_on_time must be of the same type as the pulses");
//        }
//
//        if (pulse_off_time && pulse_off_time->get_type() != m_type) {
//            throw ParameterError("pulse_off_time must be of the same type as the pulses");
//        }

        auto pulse_id = TriggerIds::new_or(id);

        m_pulses.append(Pulse(pulse_id, pulse_on_time, pulse_off_time));

        return Trigger::with_manual_id(Trigger::Type::pulse_on, pulse_id);
    }


    Voice<Trigger> flush() override {
        return m_pulses.drain()
                .template as_type<Trigger>([](const Pulse& p) {
                    return Trigger::pulse_off(p.get_id());
                });
    }

    Vec<Pulse> drain_elapsed(const TimePoint& time, bool include_missing_pulse_offs = false) {
        return m_pulses.filter_drain([time, include_missing_pulse_offs](const Pulse& p) {
            return !(p.elapsed(time) || (include_missing_pulse_offs && !p.has_pulse_off()));
        });
    }

    Voice<Trigger> drain_elapsed_as_triggers(const TimePoint& time, bool include_missing_pulse_offs = false) {
        return drain_elapsed(time, include_missing_pulse_offs)
                .template as_type<Trigger>([](const Pulse& p) {
                    return Trigger::pulse_off(p.get_id());
                });
    }

    Voice<Trigger> reset() {
        return flush();
    }

//    void set_type(DomainType type, const TimePoint& last_transport_time) {
//        if (m_type == type) {
//            return;
//        }
//
//        for (Pulse& p : m_pulses) {
//            p.set_type(type, last_transport_time);
//        }
//
//        m_type = type;
//    }

//    DomainType get_type() const { return m_type; }

    const Vec<Pulse>& vec() const { return m_pulses; }

    Vec<Pulse>& vec_mut() { return m_pulses; }


private:
    Vec<Pulse> m_pulses;
//    DomainType m_type;
};


#endif //SERIALISTLOOPER_PULSE_H
