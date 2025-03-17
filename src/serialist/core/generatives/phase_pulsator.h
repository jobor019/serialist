#ifndef SERIALIST_PHASE_PULSATOR_H
#define SERIALIST_PHASE_PULSATOR_H

#include "core/temporal/trigger.h"
#include "collections/multi_voiced.h"
#include "generatives/stereotypes/base_stereotypes.h"
#include "core/temporal/phase_pulse.h"
#include "variable.h"
#include "sequence.h"


namespace serialist {
using ThresholdIndex = std::size_t;
using DurationIndex = std::size_t;

enum class ThresholdDirection { forward, backward };


inline ThresholdDirection reverse_direction(ThresholdDirection direction) {
    if (direction == ThresholdDirection::forward) {
        return ThresholdDirection::backward;
    }
    return ThresholdDirection::forward;
}


inline Phase::Direction to_phase_direction(ThresholdDirection direction) {
    if (direction == ThresholdDirection::backward) {
        return Phase::Direction::backward;
    }
    return Phase::Direction::forward;
}


struct LegatoThreshold {
    // Return true if repositioning would move the threshold past
    //   the cursor. Also, implementation is probably different for
    //   N=1 and N>1
    bool reposition(double new_legato_value, const Phase& cursor) {
        assert(new_legato_value >= 0.0);

        if (utils::equals(legato_value, new_legato_value)) {
            return false;
        }

        if (new_legato_value == 0.0) {
            return true;
        }

        // Note: maximum/minimum delta legato is +/- 1.99(999...)
        auto delta_legato = new_legato_value - legato_value;

        auto new_threshold_position = expected_direction == ThresholdDirection::forward
                                          ? threshold_position + delta_legato
                                          : threshold_position - delta_legato;

        bool should_be_released = false;

        // Note: this code could be simplified to fewer blocks, but that would severely impact the readability
        if (delta_legato >= 1.0) {
            // Increment equal to or more than one full cycle. In this scenario, the legato threshold will always move
            //   past the cursor exactly once, since we know that the legato threshold always is ahead of the cursor
            //   (otherwise it would already have been released).
            //
            // For example,
            //   - Given expected_direction = forward, old_legato = 0.0001 and new_legato = 1.9999, we know that the
            //        cursor is in [0.0, 0.0001), otherwise the legato threshold would already have been crossed.
            //   - Similarly, expected_direction = backward, old_legato = 0.0001, and new_legato = 1.9999 would mean
            //        that the cursor always is in (0.9999, 1).
            //
            // Also note that we cannot use Phase::contains here since e.g. a change from legato 0.1 to 1.2 will be
            //   treated as Phase(0.1) -> Phase(0.2)), which for example does not contain a cursor Phase(0.0).
            has_remaining_passes = true;
        } else if (delta_legato > 0.0) {
            // Increment <1.0 (delta between 0 and 1): This case can move past the cursor 0 or 1 time in the incremental
            //   direction (same direction as expected_direction), where we need to alter has_remaining_passes in the
            //   latter scenario
            auto incremental_direction = to_phase_direction(expected_direction);
            if (Phase::contains(threshold_position, new_threshold_position, cursor, incremental_direction)) {
                has_remaining_passes = true;
            }
        } else if (utils::equals(delta_legato, -1.0)) {
            // Decrement = -1.0. Since decrements around 1.0 can lead to rounding errors, this needs its own case.
            //
            // For example, a decrement from 1.4 (1.3999999999999999) to 0.4 (0.40000000000000002)
            //   will yield a delta -0.99999999999999988, but Phase::contains(Phase(1.4), Phase(0.4)) will only
            //   be true if the cursor is exactly 0.4.
            if (has_remaining_passes) {
                has_remaining_passes = false;
            } else {
                should_be_released = true;
            }
        } else if (delta_legato > -1.0) {
            // Decrement <1.0 (delta between -1 and 0): This case can move past the cursor 0 or 1 time in the
            //   decremental direction (opposite direction of expected_direction), where we need to alter
            //   has_remaining_passes or release the threshold in the latter scenario
            auto decremental_direction = to_phase_direction(reverse_direction(expected_direction));
            if (Phase::contains(threshold_position, new_threshold_position, cursor, decremental_direction, true, false)) {
                if (has_remaining_passes) {
                    has_remaining_passes = false;
                } else {
                    should_be_released = true;
                }
            }
        } else if (utils::equals(delta_legato, -2.0)) {
            // Decrements = -2.0. Once again, needs its own case as it is subject to rounding errors
            should_be_released = true;
        } else {
            // Decrement >= 1.0: This case can move past the cursor 1 or 2 times in the negative direction. Note that
            //   Phase::contains is a valid strategy here since we're only interested in the fractional part
            //
            // For example,
            //   - Given expected_direction = forward, old_legato = 1.2 and new_legato = 0.1, we only need to check
            //       if the cursor is in the interval (0.1 -> 0.2), in which case it should be decremented twice
            //       (i.e. always released)
            auto decremental_direction = to_phase_direction(reverse_direction(expected_direction));
            if (Phase::contains(threshold_position, new_threshold_position, cursor, decremental_direction)) {
                should_be_released = true;
            } else {
                if (has_remaining_passes) {
                    has_remaining_passes = false;
                } else {
                    should_be_released = true;
                }
            }
        }

        legato_value = new_legato_value;
        threshold_position = new_threshold_position;
        return should_be_released;
    }


    void flip(const ThresholdDirection& new_direction) {
        if (new_direction != expected_direction) {
            expected_direction = new_direction;
            threshold_position.invert();
        }
    }


    Trigger trigger() const { return Trigger::pulse_off(trigger_id); }


    std::size_t trigger_id;
    double legato_value;

    Phase threshold_position;

    // ThresholdIndex pulse_on_threshold;
    // ThresholdIndex associated_threshold; // Different if legato > 1.0

    // Only relevant for N=1
    bool has_remaining_passes = false;
    // Only relevant for N=1, but should be handled in N>1 too since
    //   existing legato point might change from N>1 to N=1 on change of dur
    ThresholdDirection expected_direction;
};


struct PhasePulsatorState {
    std::optional<Phase> previous_cursor = std::nullopt;

    std::optional<LegatoThreshold> previous_legato_threshold = std::nullopt;
    std::optional<LegatoThreshold> current_legato_threshold = std::nullopt;

    // Only used by MultiThresholdStrategy
    std::optional<ThresholdIndex> last_threshold = std::nullopt;
    std::optional<ThresholdIndex> expected_next_threshold = std::nullopt;
    std::optional<DurationIndex> current_segment = std::nullopt;

    // Only used by SingleThresholdStrategy
    std::optional<ThresholdDirection> expected_direction = std::nullopt;


    Voice<Trigger> reposition(double new_legato_value, const Phase& cursor) {
        if (!current_legato_threshold) {
            return {};
        }

        if (current_legato_threshold->reposition(new_legato_value, cursor)) {
            Voice<Trigger> released;

            // We should never reposition the previous threshold, but in the case that the current threshold is released
            //   due to a reposition, the previous threshold should be released as well, since the previous pulse
            //   should never be held past the duration of the current pulse
            //
            // Note that this is added before the current, to ensure that the order of the output vector is correct

            if (previous_legato_threshold) {
                released.append(previous_legato_threshold->trigger());
                previous_legato_threshold = std::nullopt;
            }

            released.append(current_legato_threshold->trigger());
            current_legato_threshold = std::nullopt;

            return released;
        }

        return {};
    }


    Voice<Trigger> flush() {
        Voice<Trigger> triggers;

        if (previous_legato_threshold) {
            triggers.append(previous_legato_threshold->trigger());
            previous_legato_threshold = std::nullopt;
        }

        if (current_legato_threshold) {
            triggers.append(current_legato_threshold->trigger());
            current_legato_threshold = std::nullopt;
        }

        return triggers;
    }
};


// ==============================================================================================

struct PhasePulsatorParameters {
    static const inline Vec<double> DEFAULT_DURATIONS{1.0};
    static constexpr double DEFAULT_LEGATO = 1.0;

    static constexpr double LEGATO_LOWER_BOUND = 0.0;
    static constexpr double LEGATO_UPPER_BOUND = 2.0 - 1e-8;


    explicit PhasePulsatorParameters(const Vec<double>& d = DEFAULT_DURATIONS) : new_durations(d) {
        update_durations();
    }


    void update_durations() {
        if (!new_durations)
            return;

        // negative durations d correspond to pauses with duration abs(d)
        is_pause = new_durations->as_type<bool>([](double d) { return d <= 0.0; });
        durations = new_durations->map([](double d) { return std::abs(d); });

        // Normalize to 1.0
        auto sum = durations.sum();
        assert(sum > 0.0);
        durations.map([sum](double d) { return d / sum; });

        new_durations = std::nullopt;
    }


    Vec<double> durations; // should not contain negative values
    Vec<bool> is_pause;
    std::optional<Voice<double>> new_durations; // may contain negative values
    bool apply_duration_change_immediately = false;

    Vec<double> thresholds;
    double legato = DEFAULT_LEGATO;
    std::optional<double> new_legato;
};


// ==============================================================================================

class PhasePulsatorStrategies {
public:
    PhasePulsatorStrategies() = delete;


    // static ThresholdIndex next_expected(ThresholdIndex crossed_threshold, Phase::Direction);


    // static bool PhasePulsatorStrategies::is_jump_to_threshold(ThresholdIndex threshold
    //                                                           , const Phase& cursor
    //                                                           , const PhasePulsatorState& s
    //                                                           , const PhasePulsatorParameters& p);
};


// ==============================================================================================

class SingleThresholdStrategy {
public:
    static constexpr double JUMP_DETECTION_THRESHOLD = 0.3;
    static constexpr double THRESHOLD_PROXIMITY = 1e-3;

    using State = PhasePulsatorState;
    using Params = PhasePulsatorParameters;

    SingleThresholdStrategy() = delete;


    static Voice<Trigger> process(const Phase& cursor, State& s, const Params& p) {
        Voice<Trigger> triggers;

        if (!s.previous_cursor) {
            triggers.extend(s.flush());
        }

        if (*s.previous_cursor == cursor) {
            return {};
        }

        triggers.extend(process_legato_thresholds(cursor, s));

        if (detect_jump_to_threshold(cursor, s)) {
            triggers.extend(continuous_jump(cursor, s, p));
        } else if (crosses_threshold(cursor, s)) {
            triggers.extend(handle_threshold_crossing(cursor, s, p));
        }

        s.previous_cursor = cursor;
        return triggers;
    }


    static Voice<Trigger> on_activate(State&, const Params&) {
        // TODO: ignored until we implement MultiThresholdStrategy
        //   convert last_threshold / expected_next_threshold / current_segment to expected_direction
        //   handle legato thresholds
        return {};
    }


    static Voice<Trigger> handle_legato_change(State& s, const Params& p) {
        std::cout << "legato change\n";
        if (s.previous_cursor) {
            return s.reposition(p.legato, *s.previous_cursor);
        } else {
            return s.flush();
        }
    }

private:
    static Voice<Trigger> continuous_jump(const Phase& cursor, State& s, const Params& p) {
        std::cout << "continuous jump\n";
        return trigger_pulse(cursor, s, p);
    }


    static Voice<Trigger> handle_threshold_crossing(const Phase& cursor, State& s, const Params& p) {
        std::cout << "threshold crossing\n";
        if (auto crossing_direction = direction(cursor);
            !s.expected_direction || crossing_direction == *s.expected_direction) {
            return trigger_pulse(cursor, s, p);
        } else {
            flip_legato_thresholds(crossing_direction, s);
            s.expected_direction = crossing_direction;
            return {};
        }
    }


    static Voice<Trigger> process_legato_thresholds(const Phase& cursor, State& s) {
        if (!s.previous_cursor)
            return {};

        Voice<Trigger> triggers;

        auto process_threshold = [&](std::optional<LegatoThreshold>& threshold) {
            if (!threshold) return;
            auto& t = threshold.value();
            if (Phase::contains_directed(*s.previous_cursor
                                         , cursor
                                         , t.threshold_position
                                         , to_phase_direction(t.expected_direction)
                                         , std::nullopt
                                         , true
                                         , false)) {
                if (t.has_remaining_passes) {
                    t.has_remaining_passes = false;
                } else {
                    triggers.append(t.trigger());
                    threshold = std::nullopt;
                }
            }
        };

        process_threshold(s.previous_legato_threshold);
        process_threshold(s.current_legato_threshold);

        // Ensure previous threshold is not extended past current
        if (!s.current_legato_threshold && s.previous_legato_threshold) {
            // insert at front to ensure that the outgoing triggers are sorted
            triggers.insert(0, s.previous_legato_threshold->trigger());
            s.previous_legato_threshold = std::nullopt;
        }

        return triggers;
    }


    static Voice<Trigger> trigger_pulse(const Phase& cursor, State& s, const Params& p) {
        std::cout << "triggering new pulse\n";
        Voice<Trigger> triggers;

        // Release previous threshold (We're never supposed to hold more than two pulses at a time)
        if (s.previous_legato_threshold) {
            triggers.append(s.previous_legato_threshold->trigger());
            s.previous_legato_threshold = std::nullopt;
        }

        // Release current threshold or move it to previous
        if (s.current_legato_threshold) {
            if (s.current_legato_threshold->legato_value <= 1.0 || p.legato == 0.0) {
                triggers.append(s.current_legato_threshold->trigger());
            } else {
                s.previous_legato_threshold = s.current_legato_threshold;
            }
            s.current_legato_threshold = std::nullopt;
        }

        // Generate new pulse
        auto crossing_direction = direction(cursor);
        auto pulse_on = Trigger::pulse_on();
        triggers.append(pulse_on);
        s.expected_direction = crossing_direction;

        if (p.legato > 0.0) {
            auto legato_fraction = utils::modulo(p.legato, 1.0);
            s.current_legato_threshold = LegatoThreshold{
                pulse_on.get_id()
                , p.legato
                , crossing_direction == ThresholdDirection::forward
                      ? Phase(legato_fraction)
                      : Phase(1.0 - legato_fraction)
                , p.legato > 1.0
                , crossing_direction
            };
        } else {
            triggers.append(Trigger::pulse_off(pulse_on.get_id()));
        }

        return triggers;
    }


    static void flip_legato_thresholds(const ThresholdDirection& new_direction, State& s) {
        std::cout << "flipping thresholds\n";
        if (s.previous_legato_threshold) {
            s.previous_legato_threshold->flip(new_direction);
        }

        if (s.current_legato_threshold) {
            s.current_legato_threshold->flip(new_direction);
        }
    }


    static bool detect_jump_to_threshold(const Phase& cursor, const State& s) {
        // First value received
        if (!s.previous_cursor) {
            return close_to_threshold(cursor);
        }

        return close_to_threshold(cursor) && s.previous_cursor->distance_to(cursor) > JUMP_DETECTION_THRESHOLD;
    }


    static bool crosses_threshold(const Phase& cursor, const State& s) {
        if (!s.previous_cursor)
            return false;

        return Phase::wraps_around(*s.previous_cursor, cursor);
    }


    static bool close_to_threshold(const Phase& cursor) {
        return cursor.distance_to(0.0) < THRESHOLD_PROXIMITY;
    }


    static ThresholdDirection direction(const Phase& current_cursor) {
        // Since there's only threshold, if we know that we've crossed the threshold, we can safely assume that
        // the direction we've crossed in is the direction given by the value we're closest to of (0.0, 1.0)
        if (current_cursor.get() < 0.5) {
            return ThresholdDirection::forward;
        }
        return ThresholdDirection::backward;
    }
};


// ==============================================================================================

// TODO: This class might be entirely redundant.
//   It will be preserved for now, in case testing proves that it's needed. But if not, it should be removed and the
//   code should be significantly refactored (could be changed to a single class without the need for the
//   State-Strategy-Parameter pattern at all.

// class MultiThresholdStrategy {
// public:
//     static constexpr double JUMP_PROXIMITY_THRESHOLD = 1e-3;
//
//     using State = PhasePulsatorState;
//     using Params = PhasePulsatorParameters;
//
//     SingleThresholdStrategy() = delete;
//
//
//     static Voice<Trigger> process(const Phase& cursor, State& s, const Params& p) {
//         Voice<Trigger> triggers;
//
//         auto threshold = threshold_close_to(c);
//
//         if (is_jump_to_threshold(threshold, s)) {
//             triggers.extend(handle_continuous_jump(*threshold));
//         } else if (auto segment = segment_changed(cursor, s)) {
//             if (is_threshold_crossing(*segment, s)) {
//                 triggers.extend(handle_threshold_crossing(*segment));
//             } else {
//                 triggers.extend(handle_discontinuous_jump(*segment));
//             }
//         }
//
//         triggers.extend(process_legato_thresholds(cursor));
//
//         s.previous_cursor = cursor;
//         return triggers;
//     }
//
//
//     static Voice<Trigger> on_activate(State& s, const Params& p);
//     static Voice<Trigger> handle_legato_change(State& s, const Params& p);
//
// private:
//     static bool is_jump_to_threshold(const std::optional<ThresholdIndex> threshold, const State& s) {
//         return threshold && !is_adjacent(*threshold, s);
//     }
//
//
//     static bool is_threshold_crossing(const DurationIndex segment, const State& s) {
//         return is_adjacent(segment);
//     }
//
//
//     static std::optional<SegmentIndex> detect_jump_to_segment(cursor, const State& s) {}
//
//
//     static std::optional<ThresholdIndex> detect_threshold_crossing(cursor, const State& s);
//
//     // State Mutators
//     static Voice<Trigger> handle_continuous_jump(ThresholdIndex threshold, State& s, const Params& p);
//     static Voice<Trigger> handle_discontinuous_jump(SegmentIndex segment, State& s, const Params& p);
//
//
//     static Voice<Trigger> handle_threshold_crossing(SegmentIndex segment, State& s, const Params& p) {
//         auto threshold = threshold_crossed(s.current_segment, segment);
//         if (is_same_as_last(threshold)) {
//             flip_legato_thresholds(segment, threshold);
//             return {};
//
//             // TODO .. (and don't forget to handle pauses)
//         }
//     }
//
//
//     static process_legato_thresholds(const Phase& cursor, State& s);
//     void flip_legato_thresholds(SegmentIndex new_segment
//                                 , ThresholdIndex threshold_crossed
//                                 , State& s
//                                 , const Params& p);
//
//     static std::optional<ThresholdIndex> threshold_close_to(const Phase& cursor, const State& s);
//     static bool is_adjacent(ThresholdIndex threshold, const State& s);
//     static bool is_same_as_last(ThresholdIndex threshold, const State& s);
//
//     static std::optional<DurationIndex> segment_changed(const Phase& cursor);
//     static DurationIndex segment_of(const Phase& cursor, const Params& p);
//     static bool is_adjacent(SegmentIndex);
// };


// ==============================================================================================

class PhasePulsator : public Flushable<Trigger> {
public:
    Voice<Trigger> process(const Phase& cursor) {
        assert_invariants();

        Voice<Trigger> triggers;

        if (duration_change_applicable(cursor)) {
            triggers.extend(handle_duration_change());
        }

        if (m_parameters.new_legato) {
            triggers.extend(handle_legato_change());
        }

        triggers.extend(process_cursor(cursor));
        return triggers;
    }


    void set_legato(double legato) {
        m_parameters.new_legato = utils::clip(legato
                                              , PhasePulsatorParameters::LEGATO_LOWER_BOUND
                                              , PhasePulsatorParameters::LEGATO_UPPER_BOUND);
    }


    void set_durations(const Vec<double>& durations) {
        assert(!durations.empty());
        m_parameters.new_durations = durations;
    }


    Voice<Trigger> flush() override {
        return m_state.flush();
    }


    Voice<Trigger> handle_time_skip(const TimePoint&) {
        m_state.previous_cursor = std::nullopt;
        return flush();
    }

private:
    Voice<Trigger> handle_duration_change() {
        assert(m_parameters.new_durations);
        assert(!m_parameters.new_durations->empty());

        auto new_size = m_parameters.new_durations->size();
        auto old_size = m_parameters.durations.size();

        m_parameters.update_durations();

        if (new_size == 1 && old_size != 1) {
            return SingleThresholdStrategy::on_activate(m_state, m_parameters);
        } else if (new_size > 1 && old_size == 1) {
            throw std::runtime_error("MultiThresholdStrategy::on_activate: not implemented");
            // return MultiThresholdStrategy::on_activate(m_state, m_parameters);
        }
        return {}; // Change from N > 1 to M > 1
    }


    Voice<Trigger> handle_legato_change() {
        assert(m_parameters.new_legato);

        m_parameters.legato = *m_parameters.new_legato;
        m_parameters.new_legato = std::nullopt;

        if (m_parameters.durations.size() == 1) {
            return SingleThresholdStrategy::handle_legato_change(m_state, m_parameters);
        } else {
            throw std::runtime_error("MultiThresholdStrategy::handle_legato_change: not implemented");
            // return MultiThresholdStrategy::handle_legato_change(m_state, m_parameters);
        }
    }


    Voice<Trigger> process_cursor(const Phase& cursor) {
        if (m_parameters.durations.size() == 1) {
            return SingleThresholdStrategy::process(cursor, m_state, m_parameters);
        } else {
            throw std::runtime_error("MultiThresholdStrategy::process: not implemented");
            // return MultiThresholdStrategy::process(cursor, m_state, m_parameters);
        }
    }


    bool legato_changed() const { return static_cast<bool>(m_parameters.new_legato); }


    bool duration_change_applicable(const Phase& cursor) const {
        return m_parameters.new_durations
               && (m_parameters.apply_duration_change_immediately
                   || (m_state.previous_cursor
                       && (Phase::wraps_around(*m_state.previous_cursor, cursor)
                           // TODO: This would only be relevant for MultiThreshold
                           // || PhasePulsatorStrategies::is_jump_to_threshold(0, cursor, m_state, m_parameters)
                       )
                   ));
    }


    void assert_invariants() const {
        // previous legato threshold should never exist if current legato threshold doesn't
        assert(!m_state.previous_legato_threshold || m_state.current_legato_threshold);
    }


    PhasePulsatorState m_state;
    PhasePulsatorParameters m_parameters;
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
        : PulsatorBase(identifier, parent, enabled, num_voices, Keys::CLASS_NAME)
        , m_durations(add_socket(Keys::DURATION, durations))
        , m_legato(add_socket(Keys::LEGATO_AMOUNT, legato))
        , m_cursor(add_socket(Keys::CURSOR, cursor)) {}


    void set_cursor(Node<Facet>* cursor) const { m_cursor = cursor; }

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
                    .firsts_or(PhasePulsatorParameters::DEFAULT_LEGATO);
            pulsators().set(&PhasePulsator::set_legato, std::move(legato));
        }

        if (size_has_changed || m_durations.has_changed()) {
            // Note: We do not handle polyphonic sequences. If we need multiple sequences synchronized to a single
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
            triggers[i] = p[i].process(cursors[i]);
        }

        return triggers;
    }


    Socket<Facet>& m_durations;
    Socket<Facet>& m_legato;
    Socket<Facet>& m_cursor;
    //    Socket<Facet>& m_legato_type; // TODO: implement types: absolute / relative legato
};


// ==============================================================================================

template<typename FloatType = double>
struct PhasePulsatorWrapper {
    using Keys = PhasePulsatorNode::Keys;

    ParameterHandler parameter_handler;

    Sequence<Facet, FloatType> duration{Keys::DURATION
                                        , parameter_handler
                                        , Voices<FloatType>::transposed(PhasePulsatorParameters::DEFAULT_DURATIONS)
    };
    Sequence<Facet, FloatType> legato_amount{Keys::LEGATO_AMOUNT
                                             , parameter_handler
                                             , Voices<FloatType>::singular(PhasePulsatorParameters::DEFAULT_LEGATO)
    };
    Sequence<Facet, FloatType> cursor{Keys::CURSOR, parameter_handler, Voices<FloatType>::singular(0.0)};

    Sequence<Facet, bool> enabled{param::properties::enabled, parameter_handler, Voices<bool>::singular(true)};
    Variable<Facet, std::size_t> num_voices{param::properties::num_voices, parameter_handler, 0};

    PhasePulsatorNode pulsator_node{Keys::CLASS_NAME
                                    , parameter_handler
                                    , &duration
                                    , &legato_amount
                                    , &cursor
                                    , &enabled
                                    , &num_voices
    };
};
} // namespace serialist

#endif //SERIALIST_PHASE_PULSATOR_H
