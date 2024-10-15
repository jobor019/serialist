
#ifndef SERIALIST_PHASE_PULSE_H
#define SERIALIST_PHASE_PULSE_H

#include "trigger.h"
#include "collections/multi_voiced.h"
#include "phase.h"

namespace serialist {


class PhasePulse {
public:
    explicit PhasePulse(std::size_t id, double duration) : m_id(id), m_duration(duration) {
        assert(duration >= 0.0);
    }

    void increment_phase(double delta) { m_elapsed_duration += delta; }

    void scale(double factor) {
        assert(factor >= 0.0);
        m_duration *= factor;
    }

    void flag_discontinuity() { m_discontinuity_occurred = true; }

    bool has_discontinuity_flag() const { return m_discontinuity_occurred; }

    bool elapsed() const { return m_elapsed_duration >= m_duration; }

    std::size_t get_id() const { return m_id; }

private:
    std::size_t m_id;
    double m_duration;
    double m_elapsed_duration = 0.0;
    bool m_discontinuity_occurred = false;
};


// ==============================================================================================


class PhasePulses : public Flushable<Trigger> {
public:
    PhasePulses() = default;

    Trigger new_pulse(double duration, std::optional<std::size_t> id = std::nullopt) {
        auto pulse_id = TriggerIds::new_or(id);
        m_pulses.append(PhasePulse(pulse_id, duration));
        return Trigger::with_manual_id(Trigger::Type::pulse_on, pulse_id);
    }


    Voice<Trigger> flush() override {
        return m_pulses.drain()
                .template as_type<Trigger>([](const PhasePulse& p) {
                    return Trigger::pulse_off(p.get_id());
                });
    }


    Vec<PhasePulse> drain_elapsed(bool exclude_discontinuous = false) {
        return m_pulses.filter_drain([&exclude_discontinuous](const PhasePulse& p) {
            bool elapsed = p.elapsed();
            bool should_be_excluded = exclude_discontinuous && p.has_discontinuity_flag();
            return !elapsed || should_be_excluded;
        });
    }

    Voice<Trigger> drain_elapsed_as_triggers(bool exclude_discontinuous = false) {
        return drain_elapsed(exclude_discontinuous)
                .template as_type<Trigger>([](const PhasePulse& p) {
                    return Trigger::pulse_off(p.get_id());
                });
    }


    Vec<PhasePulse> drain_discontinuous() {
        return m_pulses.filter_drain([](const PhasePulse& p) {
            return !p.has_discontinuity_flag();
        });
    }


    Voice<Trigger> drain_discontinuous_as_triggers() {
        return drain_discontinuous()
                .template as_type<Trigger>([](const PhasePulse& p) {
                    return Trigger::pulse_off(p.get_id());
                });
    }


    void increment_phase(double delta) {
        for (auto& pulse: m_pulses) {
            pulse.increment_phase(delta);
        }
    }


    void scale(double factor) {
        for (auto& pulse: m_pulses) {
            pulse.scale(factor);
        }
    }


    void flag_discontinuity() {
        for (auto& pulse: m_pulses) {
            pulse.flag_discontinuity();
        }
    }

    const Vec<PhasePulse>& vec() const { return m_pulses; }

    Vec<PhasePulse>& vec_mut() { return m_pulses; }


private:
    Vec<PhasePulse> m_pulses;
};


} // namespace serialist

#endif //SERIALIST_PHASE_PULSE_H
