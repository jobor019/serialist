
#ifndef SERIALISTLOOPER_PULSE_H
#define SERIALISTLOOPER_PULSE_H

#include <optional>
#include "core/types/trigger.h"
#include "core/types/time_point.h"
#include "core/collections/vec.h"
#include "core/collections/held.h"
#include "core/exceptions.h"

namespace serialist {

struct PulseIdentifier {
    std::size_t id;
    bool triggered = false;

    bool operator==(const PulseIdentifier& other) const { return id == other.id; }
};


// ==============================================================================================

class Pulse {
public:
    Pulse(std::size_t id, const DomainTimePoint& trigger_time
          , const std::optional<DomainTimePoint>& pulse_off_time = std::nullopt)
            : m_id(id)
              , m_trigger_time(trigger_time)
              , m_pulse_off_time(pulse_off_time) {
        assert_invariants();
    }

    bool elapsed(const TimePoint& current_time) const {
        return m_pulse_off_time && m_pulse_off_time->elapsed(current_time);
    }

    bool has_pulse_off() const {
        return static_cast<bool>(m_pulse_off_time);
    }

    std::size_t get_id() const { return m_id; }

    /** @throws ParameterError if pulse_off_time is not of the same type as the trigger time */
    void set_pulse_off(const DomainTimePoint& time) {
        if (time.get_type() != m_trigger_time.get_type()) {
            throw ParameterError("pulse_off_time must be of the same type as the trigger time");
        }
        m_pulse_off_time = time;
    }


    void scale_duration(double factor) {
        if (m_pulse_off_time) {
            auto duration = m_trigger_time - *m_pulse_off_time;
            m_pulse_off_time = m_trigger_time + (duration * factor);
        }
    }

    /**
     * @brief sets period of the pulse if period is of the same type as the trigger time
     */
    bool try_set_duration(const DomainDuration& duration) {
        if (m_trigger_time.get_type() == duration.get_type()) {
            m_pulse_off_time = m_trigger_time + duration;
            return true;
        }
        return false;
    }

    DomainTimePoint get_trigger_time() const { return m_trigger_time; }

    const std::optional<DomainTimePoint>& get_pulse_off_time() const { return m_pulse_off_time; }

    DomainType get_type() const {
        return m_trigger_time.get_type();
    }

private:
    void assert_invariants() {
        assert(!m_pulse_off_time || m_pulse_off_time->get_type() == m_trigger_time.get_type());
    }

    std::size_t m_id;
    DomainTimePoint m_trigger_time;
    std::optional<DomainTimePoint> m_pulse_off_time;
};


// ==============================================================================================


class Pulses : public Flushable<Trigger> {
public:

    Trigger new_pulse(const DomainTimePoint& pulse_on_time
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

    Vec<Pulse> drain_elapsed(const TimePoint& time, bool include_endless = false) {
        return m_pulses.filter_drain([time, include_endless](const Pulse& p) {
            return !(p.elapsed(time) || (include_endless && !p.has_pulse_off()));
        });
    }

    Voice<Trigger> drain_elapsed_as_triggers(const TimePoint& time, bool include_endless = false) {
        return drain_elapsed(time, include_endless)
                .template as_type<Trigger>([](const Pulse& p) {
                    return Trigger::pulse_off(p.get_id());
                });
    }

    Voice<Pulse> drain_endless() {
        return m_pulses.filter_drain([](const Pulse& p) {
            return !p.has_pulse_off();
        });
    }

    Voice<Pulse> drain_by_id(std::size_t id) {
        return m_pulses.filter_drain([id](const Pulse& p) {
            return p.get_id() != id; // remove all elements for which the id matches
        });
    }

    Voice<Trigger> drain_by_id_as_triggers(std::size_t id) {
        return drain_by_id(id)
                .template as_type<Trigger>([](const Pulse& p) {
                    return Trigger::pulse_off(p.get_id());
                });
    }

    Voice<Trigger> drain_endless_as_triggers() {
        return drain_endless()
                .template as_type<Trigger>([](const Pulse& p) {
                    return Trigger::pulse_off(p.get_id());
                });
    }

    Voice<Pulse> drain_non_matching(DomainType type) {
        return m_pulses.filter_drain([&type](const Pulse& p) {
            return p.get_trigger_time().get_type() == type; // remove all elements for which the type does not match
        });
    }

    Voice<Trigger> drain_non_matching_as_triggers(DomainType type) {
        return drain_non_matching(type)
                .template as_type<Trigger>([](const Pulse& p) {
                    return Trigger::pulse_off(p.get_id());
                });
    }



    void scale_durations(double factor) {
        for (Pulse& p : m_pulses) {
            p.scale_duration(factor);
        }
    }

    void try_set_durations(const DomainDuration& duration) {
        for (Pulse& p : m_pulses) {
            p.try_set_duration(duration);
        }
    }



    const Pulse* last_of_type(DomainType type) const {
        const Pulse* output = nullptr;
        for (const auto& p: m_pulses) {
            if (p.get_trigger_time().get_type() == type && p.has_pulse_off()) {
                if (!output || output->get_pulse_off_time()->get_value() < p.get_pulse_off_time()->get_value()) {
                    output = &p;
                }
            }
        }
        return output;
    }



//    /**
//     * @brief Sets period for all existing pulses to fixed value
//     */
//    void reschedule(const DomainDuration& new_duration) {
//        for (Pulse& p : m_pulses) {
//            // TODO: This is not a good strategy: what if initial DTP is incompatible with new_duration?
//            p.set_pulse_off(p.get_trigger_time() + new_duration);
//        }
//    }

    Voice<Trigger> reset() {
        return flush();
    }

    bool empty() const {
        return m_pulses.empty();
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


// ==============================================================================================

/**
 * @brief Tracks held pulses for multiple outlets, i.e. Vec<Voices<Trigger>>
 */
class MultiOutletHeldPulses {
public:
    using OutletHeld = MultiVoiceHeld<PulseIdentifier>;

    explicit MultiOutletHeldPulses(std::size_t num_outlets) : m_held(create_container(num_outlets)) {}


    void append_pulse_ons(Vec<Voices<Trigger>>& output) {
        throw std::runtime_error("Not implemented");
    }


    void flush_into(Vec<Voices<Trigger>>& output) {
        throw std::runtime_error("Not implemented");
    }
    void flush_into(Vec<Voices<Trigger>>& output, std::size_t outlet_index) {
        throw std::runtime_error("Not implemented");
    }
    void flush_into(Vec<Voices<Trigger>>& output, std::size_t outlet_index, std::size_t voice_index) {
        throw std::runtime_error("Not implemented");
    }

    void flush_or_flag_as_triggered(Vec<Voices<Trigger>>& output) {
        throw std::runtime_error("Not implemented");
    }
    void flush_or_flag_as_triggered(Vec<Voices<Trigger>>& output, std::size_t outlet_index) {
        throw std::runtime_error("Not implemented");
    }
    void flush_or_flag_as_triggered(Vec<Voices<Trigger>>& output, std::size_t outlet_index, std::size_t voice_index) {
        throw std::runtime_error("Not implemented");
    }

    Vec<Voices<Trigger>> flush() {
        throw std::runtime_error("Not implemented");
    }


private:
    static Vec<OutletHeld> create_container(std::size_t num_outlets) {
        return Vec<OutletHeld>::repeated(num_outlets, OutletHeld{1});
    }


    Vec<OutletHeld> m_held;
};


} // namespace serialist

#endif //SERIALISTLOOPER_PULSE_H
