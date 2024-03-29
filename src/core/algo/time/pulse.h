
#ifndef SERIALISTLOOPER_PULSE_H
#define SERIALISTLOOPER_PULSE_H

#include <optional>
#include "core/algo/time/trigger.h"
#include "core/collections/vec.h"
#include "core/collections/held.h"

template<typename TimePointType = double>
class Pulse {
public:
    Pulse(int id, TimePointType trigger_time, std::optional<TimePointType> pulse_off_time = std::nullopt)
            : m_id(id)
              , m_trigger_time(trigger_time)
              , m_pulse_off_time(pulse_off_time) {
    }

    bool elapsed(TimePointType time) const {
        return m_pulse_off_time && time >= *m_pulse_off_time;
    }

    bool has_pulse_off() const {
        return static_cast<bool>(m_pulse_off_time);
    }

    int get_id() const { return m_id; }

    void set_pulse_off(TimePointType time) {
        m_pulse_off_time = time;
    }

    TimePointType get_trigger_time() const { return m_trigger_time; }

    const std::optional<TimePointType>& get_pulse_off_time() const { return m_pulse_off_time; }

private:
    int m_id;
    TimePointType m_trigger_time;
    std::optional<TimePointType> m_pulse_off_time;
};


// ==============================================================================================


template<typename TimePointType = double>
class Pulses : public Flushable<Trigger> {
public:
    Trigger new_pulse(TimePointType pulse_on_time, std::optional<TimePointType> pulse_off_time) {
        m_last_scheduled_id = (m_last_scheduled_id + 1) % std::numeric_limits<int>::max();

        m_pulses.append(Pulse<>(m_last_scheduled_id, pulse_on_time, pulse_off_time));

        return Trigger::pulse_on(m_last_scheduled_id);
    }

    Voice<Trigger> flush() override {
        return m_pulses.drain()
                .template as_type<Trigger>([](const Pulse<TimePointType>& p) {
                    return Trigger(Trigger::Type::pulse_off, p.get_id());
                });
    }

    Vec<Pulse<TimePointType>> drain_elapsed(double time, bool include_missing_pulse_offs = false) {
        return m_pulses.filter_drain([time, include_missing_pulse_offs](const Pulse<TimePointType>& p) {
            return !(p.elapsed(time) || (include_missing_pulse_offs && !p.has_pulse_off()));
        });
    }

    Voice<Trigger> drain_elapsed_as_triggers(double time, bool include_missing_pulse_offs = false) {
        return drain_elapsed(time, include_missing_pulse_offs)
                .template as_type<Trigger>([](const Pulse<TimePointType>& p) {
                    return Trigger::pulse_off(p.get_id());
                });
    }

    Voice<Trigger> reset() {
        m_last_scheduled_id = Trigger::NO_ID;
        return flush();
    }

    const Vec<Pulse<TimePointType>>& vec() const {
        return m_pulses;
    }

    Vec<Pulse<TimePointType>>& vec_mut() {
        return m_pulses;
    }


private:
    Vec<Pulse<TimePointType>> m_pulses;

    int m_last_scheduled_id = Trigger::NO_ID;
};


#endif //SERIALISTLOOPER_PULSE_H
