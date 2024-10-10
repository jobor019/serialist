
#ifndef SERIALIST_PHASE_PULSATOR_H
#define SERIALIST_PHASE_PULSATOR_H

#include "algo/temporal/trigger.h"
#include "collections/multi_voiced.h"
#include "generatives/stereotypes/base_stereotypes.h"
#include "algo/temporal/phase_pulse.h"
#include "utility/stateful.h"

namespace serialist {

class PhasePulsator : public Flushable<Trigger> {
public:
    PhasePulsator() = default;


    Voice<Trigger> process(const TimePoint& t, const Phase& cursor) {
        // TODO
    }


    bool detect_discontinuity(const Phase& cursor) const {
        // TODO
    }


    std::optional<std::size_t> detect_edge(const TimePoint& t, const Phase& cursor) const {
        // TODO
    }


    void set_durations(const Voice<Phase>& durations) {
        // TODO
    }


    void set_legato(double legato) {
        // TODO
    }


private:
    PhasePulses  m_pulses;
    WithChangeFlag<Voice<Phase>> m_durations;
    double m_legato = 1.0;

    std::optional<Phase> m_previous_cursor = std::nullopt;
    std::optional<TimePoint> m_previous_trigger_time = std::nullopt;


};


// ==============================================================================================


class PhasePulsatorNode : public NodeBase<Trigger> {

};


// ==============================================================================================


template<typename FloatType = float>
struct PhasePulsatorWrapper {

};


} // namespace serialist

#endif //SERIALIST_PHASE_PULSATOR_H
