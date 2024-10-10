
#ifndef SERIALIST_PHASE_PULSE_H
#define SERIALIST_PHASE_PULSE_H

#include "trigger.h"
#include "collections/multi_voiced.h"
#include "phase.h"

namespace serialist {


class PhasePulse {
public:
    explicit PhasePulse(std::size_t id, const Phase& duration) : m_id(id), m_duration(duration) {}

    void increment_phase(double delta) { m_elapsed_duration += delta; }

    void scale(double factor) { m_duration *= factor; }

    void flag_discontinuity() { m_discontinuity_occurred = true; }

    bool has_discontinuity_flag() const { return m_discontinuity_occurred; }

    bool elapsed() const { return m_elapsed_duration >= m_duration.get(); }

    std::size_t get_id() const { return m_id; }

private:
    std::size_t m_id;
    Phase m_duration;
    double m_elapsed_duration = 0.0;
    bool m_discontinuity_occurred = false;
};


// ==============================================================================================


class PhasePulses : public Flushable<Trigger> {
public:
    PhasePulses() = default;

    Trigger new_pulse(const Phase& duration, std::optional<std::size_t> id = std::nullopt) {
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


    Voice<Trigger> drain_elapsed_as_triggers(bool include_discontinuous = false) {
        // TODO
    }


    Voice<Trigger> drain_discontinuous_as_triggers() {
        // TODO
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
    Vec<PhasePulse>  m_pulses;
};


} // namespace serialist

#endif //SERIALIST_PHASE_PULSE_H
