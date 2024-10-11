
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
    static constexpr double DISCONTINUITY_THRESHOLD = 0.1; // unit: phases
    static constexpr double MINIMUM_SEGMENT_DURATION = 0.03; // unit: ticks, smallest allowed duration ≈ 128d-notes

    PhasePulsator() = default;


    Voice<Trigger> process(const TimePoint& t, const Phase& cursor) {
        Voice<Trigger> triggers;

        auto direction = cursor_direction(cursor);
        if (m_durations.changed() || time_skip(t) || is_discontinuous(cursor) || direction_changed(direction)) {
            m_pulses.flag_discontinuity();
            m_durations.clear_flag();
            m_current_segment = cursor_segment(cursor);

        } else if (is_first_call()) {
            // On the first call, we don't know where in the segment we're starting, and we don't know the
            //   direction the cursor is moving. For this reason, we'll always trigger a new pulse equal to the segment
            //   duration, but flag it as discontinuous to ensure that it doesn't extend beyond the next segment.
            auto segment_index = cursor_segment(cursor);
            m_pulses.new_pulse(m_durations.get()[segment_index]);
            m_current_segment = segment_index;
            m_previous_trigger_time = t;
            m_pulses.flag_discontinuity();

        } else {
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
                m_current_segment = edge_index;
                m_previous_trigger_time = t;
            }
        }

        m_cursor_direction = direction;
        m_previous_cursor = cursor;
        m_previous_trigger_time = t;

        return triggers;
    }


    std::optional<std::size_t> detect_edge(const TimePoint& t, const Phase& cursor) const {
        if (too_close_to_previous_trigger(t) || !m_current_segment)
            return std::nullopt;

        auto new_segment_index = cursor_segment(cursor);
        if (new_segment_index != *m_current_segment)
            return new_segment_index;
        
        return std::nullopt;

    }

    std::size_t cursor_segment(const Phase& cursor) const {
        double sum = 0.0;
        for (std::size_t i = 0; i < m_durations->size(); ++i) {
            sum += m_durations.get()[i];
            if (sum > cursor.get()) {
                return i;
            }
        }
        return 0;
    }


    void set_durations(Voice<double>&& durations) {
        durations.filter_drain([](double p) { return utils::equals(p, 0.0); });

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
    bool too_close_to_previous_trigger(const TimePoint& t) const {
        // Technically, we already know that no time skip has occurred, so we don't need std::abs here.
        //   This is only in case the implementation changes in the future
        return m_previous_trigger_time &&
               std::abs(t.get_tick() - m_previous_trigger_time->get_tick()) > MINIMUM_SEGMENT_DURATION;
    }

    bool is_first_call() const {
        return m_previous_cursor == std::nullopt;
    }


    bool time_skip(const TimePoint& t) const {
        return m_previous_trigger_time && m_previous_trigger_time->get_tick() > t.get_tick();
    }

    std::optional<Phase::Direction> cursor_direction(const Phase& cursor) const {
        if (!m_previous_cursor)
            return std::nullopt;

        return Phase::direction(*m_previous_cursor, cursor);
    }


    bool is_discontinuous(const Phase& cursor) const {
        return m_previous_cursor && Phase::abs_delta_phase(*m_previous_cursor, cursor) > DISCONTINUITY_THRESHOLD;
    }


    bool direction_changed(std::optional<Phase::Direction> new_direction) const {
        if (!m_cursor_direction)
            return false;
        return *m_cursor_direction != *new_direction;
    }

    PhasePulses m_pulses;
    WithChangeFlag<Voice<double>> m_durations{Voice<double>::singular(1.0)}; // INVARIANT: Minimum size 1
    double m_legato = 1.0;

    std::optional<Phase> m_previous_cursor = std::nullopt;
    std::optional<Phase::Direction> m_cursor_direction = std::nullopt;
    std::optional<TimePoint> m_previous_trigger_time = std::nullopt;
    std::optional<std::size_t> m_current_segment = std::nullopt;
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
