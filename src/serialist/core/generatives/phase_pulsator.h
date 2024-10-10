
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
    static constexpr double DISCONTINUITY_THRESHOLD = 0.1;

    PhasePulsator() = default;


    Voice<Trigger> process(const TimePoint& t, const Phase& cursor) {
        Voice<Trigger> triggers;

        auto direction = cursor_direction(cursor);
        if (m_durations.changed() || is_discontinuous(cursor) || direction_changed(direction)) {
            m_pulses.flag_discontinuity();
            m_durations.clear_flag();

        } else if (m_previous_cursor) {
            m_pulses.increment_phase(Phase::abs_delta_phase(*m_previous_cursor, cursor));
            // For previously flagged discontinuous pulses, we'll use their original duration if
            //   (at the moment of evaluation) legato < 1.0. Otherwise (legato 1.0 or greater), we'll override their
            //   durations and extend them until the next pulse on.
            //
            // Note that a proper implementation of legato requires prediction of the future, which we cannot do,
            //   as the parameters may change at any given moment. But this way we'll ensure that for legato >= 1.0,
            //   we'll never have any gaps in the output pulses, even if the corpus / cursor / direction changes.
            bool include_discontinuous = m_legato < 1.0;
            triggers.extend(m_pulses.drain_elapsed_as_triggers(include_discontinuous));

            if (auto edge_index = detect_edge(t, cursor)) {
                triggers.extend(m_pulses.drain_discontinuous_as_triggers());
                triggers.append(m_pulses.new_pulse(m_durations.get()[*edge_index]));
            }
        } // else: first call to object, no previous cursor so no edges to detect

        m_cursor_direction = direction;
        m_previous_cursor = cursor;
        m_previous_trigger_time = t;

        return triggers;
    }


    std::optional<std::size_t> detect_edge(const TimePoint& t, const Phase& cursor) const {
        // TODO
    }


    void set_durations(Voice<double>&& durations) {
        durations.filter_drain([](double p) { return utils::equals(p, 0.0) ; });
        if (durations.empty())
            m_durations = Voice<double>::singular(1.0);

        durations.normalize_l1();
        m_durations = durations;
    }


    void set_legato(double new_legato) {
        if (!utils::equals(m_legato, 0.0)) {
            m_pulses.scale(new_legato / m_legato);
        }

        m_legato = new_legato;

    }


private:
    std::optional<Phase::Direction> cursor_direction(const Phase& cursor) const {
        if (!m_previous_cursor)
            return std::nullopt;

        return Phase::direction(*m_previous_cursor, cursor);
    }


    bool is_discontinuous(const Phase& cursor) const {
        return m_previous_cursor && Phase::abs_delta_phase(*m_previous_cursor, cursor) > DISCONTINUITY_THRESHOLD;
    }


    bool direction_changed(std::optional<Phase::Direction> new_direction) {
        if (!m_cursor_direction)
            return false;
        return *m_cursor_direction != *new_direction;
    }

    PhasePulses m_pulses;
    WithChangeFlag<Voice<double>> m_durations{Voice<double>::singular(1.0)};
    double m_legato = 1.0;

    std::optional<Phase> m_previous_cursor = std::nullopt;
    std::optional<Phase::Direction> m_cursor_direction = std::nullopt;
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
