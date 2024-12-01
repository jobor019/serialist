
#ifndef SERIALIST_PHASE_PULSATOR_H
#define SERIALIST_PHASE_PULSATOR_H

#include "algo/temporal/trigger.h"
#include "collections/multi_voiced.h"
#include "generatives/stereotypes/base_stereotypes.h"
#include "algo/temporal/phase_pulse.h"
#include "utility/stateful.h"
#include "variable.h"
#include "sequence.h"

namespace serialist {

class PhasePulsator : public Flushable<Trigger> {
public:
    static constexpr double DISCONTINUITY_THRESHOLD = 0.1; // unit: phases
    static constexpr double MINIMUM_SEGMENT_DURATION = 0.03; // unit: ticks, smallest allowed duration ≈ 128d-notes
    static constexpr double DEFAULT_LEGATO = 1.0;
    static constexpr double DEFAULT_DURATION = 1.0;

    PhasePulsator() = default;


    Voice<Trigger> process(const TimePoint& t, const Phase& cursor) {
        Voice<Trigger> triggers;
        auto direction = cursor_direction(cursor);
        if (m_previous_trigger_time && (m_durations.changed() || is_discontinuous(cursor) || direction_changed(direction))) {
            m_pulses.flag_discontinuity();
            m_current_segment = cursor_segment(cursor);

        } else if (is_first_call()) {
            // On the first call, we don't know where in the segment we're starting, and we don't know the
            //   direction the cursor is moving. For this reason, we'll always trigger a new pulse equal to the segment
            //   duration, but flag it as discontinuous to ensure that it doesn't extend beyond the next segment.
            auto segment_index = cursor_segment(cursor);
            triggers.append(new_pulse(segment_index));
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
            bool include_discontinuous = m_legato >= 1.0;
            triggers.extend(m_pulses.drain_elapsed_as_triggers(include_discontinuous));

            if (auto edge_index = detect_edge(t, cursor)) {
                // If legato is exactly 1.0, we'll flush all pulses just to make sure they end up in the same cycle
                if (utils::equals(m_legato, 1.0)) {
                    triggers.extend(m_pulses.flush());
                } else {
                    triggers.extend(m_pulses.drain_discontinuous_as_triggers());
                }

                triggers.append(new_pulse(*edge_index));
                m_current_segment = edge_index;
                m_previous_trigger_time = t;
            }
        }

        if (direction != Phase::Direction::unchanged) {
            m_cursor_direction = direction;
        }

        m_previous_cursor = cursor;
        m_durations.clear_flag();

        return triggers;
    }


    Voice<Trigger> flush() override {
        return m_pulses.flush();
    }


    Voice<Trigger> handle_time_skip(const TimePoint& new_time) {
        m_previous_trigger_time = new_time;
        return flush();
    }


    void set_durations(const Voice<double>& durations) {
        auto d = durations.cloned().filter_drain([](double p) { return p <= 0.0; });

        if (d.empty()) {
            m_durations = Voice<double>::singular(1.0);

        } else {
            d.normalize_l1();
            m_durations = d;
        }
    }


    void set_legato(double new_legato) {
        assert(new_legato >= 0.0);

        if (!utils::equals(m_legato, 0.0)) {
            m_pulses.scale(new_legato / m_legato);
        }

        m_legato = new_legato;

    }


private:
    Trigger new_pulse(std::size_t edge_index) {
        return m_pulses.new_pulse(m_durations.get()[edge_index] * m_legato);
    }


    std::optional<std::size_t> detect_edge(const TimePoint& t, const Phase& cursor) const {
        if (too_close_to_previous_trigger(t) || !m_current_segment) {
            return std::nullopt;
        }

        auto new_segment_index = cursor_segment(cursor);
        if (new_segment_index != *m_current_segment)
            return new_segment_index;

        if (m_durations->size() == 1 && std::abs(m_previous_cursor->get() - cursor.get()) > DISCONTINUITY_THRESHOLD) {
            // If the corpus consists of exactly one segment, the step from phase ~1 to phase ~0 (or vice versa) would
            // // still be considered an edge. When this function is called, we know that it's not discontinuous,
            //    meaning that any raw abs diff greater than DISCONTINUITY_THRESHOLD indicates that the cursor indeed
            //    has wrapped around, and thus should trigger an edge at the current (only) segment.
            return 0;
        }

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


    bool too_close_to_previous_trigger(const TimePoint& t) const {
        return m_previous_trigger_time &&
               std::abs(t.get_tick() - m_previous_trigger_time->get_tick()) < MINIMUM_SEGMENT_DURATION;
    }


    bool is_first_call() const {
        return m_previous_cursor == std::nullopt;
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
        if (!m_cursor_direction || new_direction == Phase::Direction::unchanged)
            return false;
        return *m_cursor_direction != *new_direction;
    }


    PhasePulses m_pulses;
    WithChangeFlag<Voice<double>> m_durations{Voice<double>::singular(DEFAULT_DURATION)}; // INVARIANT: Minimum size 1
    double m_legato = DEFAULT_LEGATO;

    std::optional<Phase> m_previous_cursor = std::nullopt;
    std::optional<Phase::Direction> m_cursor_direction = std::nullopt;
    std::optional<TimePoint> m_previous_trigger_time = std::nullopt;
    std::optional<std::size_t> m_current_segment = std::nullopt;
};


// ==============================================================================================

class PhasePulsatorNode : public PulsatorBase<PhasePulsator> {
public:
    struct Keys {
        static const inline std::string DURATION = "duration";
        static const inline std::string LEGATO_AMOUNT = "legato_amount";
        static const inline std::string CURSOR = "cursor";

        static const inline std::string CLASS_NAME = "phase_pulsator";
    };

    PhasePulsatorNode(const std::string& identifier
                      , ParameterHandler& parent
                      , Node<Facet>* durations = nullptr
                      , Node<Facet>* legato = nullptr
                      , Node<Facet>* cursor = nullptr
                      , Node<Facet>* enabled = nullptr
                      , Node<Facet>* num_voices = nullptr)
            : PulsatorBase<PhasePulsator>(identifier, parent, enabled, num_voices, Keys::CLASS_NAME)
              , m_durations(add_socket(Keys::DURATION, durations))
              , m_legato(add_socket(Keys::LEGATO_AMOUNT, legato))
              , m_cursor(add_socket(Keys::CURSOR, cursor)) {}


private:
    std::size_t get_voice_count() override {
        // Note: durations are not part of voice count, as their role is that of a corpus.
        //       More elements means a longer sequence, not more simultaneous voices.
        return voice_count(m_legato.voice_count(), m_cursor.voice_count());
    }

    std::optional<Voices<Trigger>> handle_enabled_state(EnabledState state) override {
        if (state == EnabledState::disabled_this_cycle) {
            return pulsators().flush();
        } else if (state == EnabledState::disabled_previous_cycle) {
            return Voices<Trigger>::empty_like();
        }
        return std::nullopt;
    }

    void update_parameters(std::size_t num_voices, bool size_has_changed) override {
        if (size_has_changed || m_legato.has_changed()) {
            auto legato = m_legato.process()
                    .adapted_to(num_voices)
                    .firsts_or(PhasePulsator::DEFAULT_LEGATO);
            pulsators().set(&PhasePulsator::set_legato, std::move(legato));
        }

        if (size_has_changed || m_durations.has_changed()) {
            // Note: We do not handle polyphonic sequences. If we need multiple sequences sync'ed to a single
            //       oscillator, the optimal approach is to use multiple PhasePulsator objects instead.
            auto durations = m_durations.process().firsts_or(0.0);
            pulsators().set(&PhasePulsator::set_durations, durations);
        }

        // Note: cursor should not be updated here as it's not a parameter (member variable) of PhasePulsator
    }

    std::optional<Voices<Trigger>> handle_transport_events(const TimePoint& t, const Vec<TimeEvent>& events) override {
        if (events.contains(TimeEvent::time_skip)) {
            auto& p = pulsators();
            auto num_voices = p.size();
            auto flushed = Voices<Trigger>::zeros(num_voices);
            for (std::size_t i = 0; i < num_voices; ++i) {
                flushed[i] = p[i].handle_time_skip(t);
            }
            return flushed;
        }

        return std::nullopt;

    }

    Voices<Trigger> process_pulsator(const TimePoint& t, std::size_t num_voices) override {
        auto triggers = Voices<Trigger>::zeros(num_voices);

        if (!m_cursor.is_connected()) {
            return triggers;
        }

        auto cursors = m_cursor.process()
                .adapted_to(num_voices)
                .firsts_or(0.0)
                .as_type<Phase>([](auto phase) { return Phase(phase); });

        auto& p = pulsators();
        assert(p.size() == num_voices);

        for (std::size_t i = 0; i < p.size(); ++i) {
            triggers[i] = p[i].process(t, cursors[i]);
        }

        return triggers;
    }


    Socket<Facet>& m_durations;
    Socket<Facet>& m_legato;
    Socket<Facet>& m_cursor;
//    Socket<Facet>& m_legato_type; // TODO: implement types: absolute / relative legato

};


// ==============================================================================================

template<typename FloatType = float>
struct PhasePulsatorWrapper {

    using Keys = PhasePulsatorNode::Keys;

    ParameterHandler parameter_handler;

    Sequence<Facet, FloatType> duration{Keys::DURATION, parameter_handler
                                        , Voices<FloatType>::singular(PhasePulsator::DEFAULT_DURATION)};
    Sequence<Facet, FloatType> legato_amount{Keys::LEGATO_AMOUNT, parameter_handler
                                             , Voices<FloatType>::singular(PhasePulsator::DEFAULT_LEGATO)};
    Sequence<Facet, FloatType> cursor{Keys::CURSOR, parameter_handler, Voices<FloatType>::singular(0.0)};

    Sequence<Facet, bool> enabled{param::properties::enabled, parameter_handler, Voices<bool>::singular(true)};
    Variable<Facet, std::size_t> num_voices{param::properties::num_voices, parameter_handler, 0};

    PhasePulsatorNode pulsator_node{Keys::CLASS_NAME
                                    , parameter_handler
                                    , &duration
                                    , &legato_amount
                                    , &cursor
                                    , &enabled
                                    , &num_voices};

};


} // namespace serialist

#endif //SERIALIST_PHASE_PULSATOR_H
