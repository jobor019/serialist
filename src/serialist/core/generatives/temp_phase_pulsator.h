#ifndef SERIALIST_PHASE_PULSATOR_H
#define SERIALIST_PHASE_PULSATOR_H

#include "core/collections/voices.h"
#include "temporal/phase.h"
#include "temporal/time_point.h"
#include "temporal/trigger.h"
#include "utility/stateful.h"


namespace serialist {
using ThresholdIndex = std::size_t;
using DurationIndex = std::size_t;

enum class ThresholdDirection { forward, backward };

struct DirectedModInterval {
    double m_from;
    double m_to;
    double m_modulo = 1.0;

    bool contains(double position, std::optional<ThresholdDirection> expected_direction = std::nullopt);
};

struct LegatoThreshold {
    std::size_t trigger_id;
    double legato_value;

    double threshold_position;

    ThresholdIndex pulse_on_threshold;
    ThresholdIndex associated_threshold; // Different if legato > 1.0

    // Only relevant for N=1
    bool has_remaining_passes = false;
    // Only relevant for N=1, but should be handled in N>1 too since
    //   existing legato point might change from N>1 to N=1 on change of dur
    ThresholdDirection expected_direction;


    // Return true if repositioning would move the threshold past
    //   the cursor. Also, implementation is probably different for
    //   N=1 and N>1
    bool reposition(double new_legato_value, const Phase& cursor);

    bool flip(ThresholdIndex last_threshold);

    Trigger trigger() const { return Trigger::pulse_off(trigger_id); }
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

            auto
            released.append(current_legato_threshold->trigger());
            current_legato_threshold = std::nullopt;

            return released;
        }

        return {};
    }


    Voice<Trigger> flush() {
        Voice<Trigger> triggers;

        if (previous_legato_threshold) {
            released.append(previous_legato_threshold->trigger());
            previous_legato_threshold = std::nullopt;
        }

        if (current_legato_threshold) {
            released.append(current_legato_threshold->trigger());
            current_legato_threshold = std::nullopt;
        }

        return released;
    }
};


struct PhasePulsatorParameters {
    Vec<double> durations; // should not contain negative values
    Vec<bool> is_pause;
    std::optional<Voice<double>> new_durations; // may contain negative values
    bool apply_duration_change_immediately = false;

    Vec<double> thresholds;
    double legato;
    std::optional<double> new_legato;
};


class PhasePulsatorStrategies {
public:
    PhasePulsatorStrategies = delete;


    static ThresholdIndex next_expected(ThresholdIndex crossed_threshold, Phase::Direction);


    static bool is_wrap_around(const Phase& cursor_from, const Phase& cursor_to) {
        return DirectedModInterval{cursor_from.get(), cursor_to.get()}.contains(0.0);
    }


    static bool PhasePulsatorStrategies::is_jump_to_threshold(ThresholdIndex threshold
                                                              , const Phase& cursor
                                                              , const PhasePulsatorState& s
                                                              , const PhasePulsatorParameters& p);
}


class SingleThresholdStrategy {
public:
    static constexpr double JUMP_DETECTION_THRESHOLD = 0.3;
    static constexpr double THRESHOLD_PROXIMITY = 1e-3;

    using State = PhasePulsatorState;
    using Params = PhasePulsatorParameters;

    SingleThresholdStrategy() = delete;


    static Voice<Trigger> process(const Phase& cursor, State& s, const Params& p) {
        Voice<Trigger> triggers;

        if (detect_jump_to_threshold(cursor, s, p)) {
            triggers.extend(continuous_jump(cursor, s, p));
        } else if (crosses_threshold(cursor, s, p)) {
            triggers.extend(handle_threshold_crossing(cursor, s, p));
        }

        triggers.extend(process_legato_thresholds(cursor, s, p));

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
        if (s.previous_cursor) {
            return s.reposition(p.legato, *s.previous_cursor);
        } else {
            return s.flush();
        }
    }

private:
    static Voice<Trigger> continuous_jump(const Phase& cursor, State& s, const Params& p) {
        return trigger_pulse(cursor, s, p);
    }


    static Voice<Trigger> handle_threshold_crossing(const Phase& cursor, State& s, const Params& p) {
        if (auto crossing_direction = direction(cursor);
            !s.expected_direction || crossing_direction == *s.expected_direction) {
            return trigger_pulse(cursor, s, p);
        } else {
            flip_legato_thresholds(crossing_direction, s, p);
            return {};
        }
    }


    static Voice<Trigger> process_legato_thresholds(const Phase& cursor, State& s, const Params& p) {
        if (!s.previous_cursor)
            return s.flush();

        auto transition_interval = DirectedModInterval{s.previous_cursor->get(), cursor.get()};
        Voice<Trigger> triggers;

        auto process_threshold = [&](std::optional<LegatoThreshold>& threshold) {
            if (!threshold) return;
            auto& t = threshold.value();
            if (transition_interval.contains(t.threshold_position, t.expected_direction)) {
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
            triggers.append(s.previous_legato_threshold->trigger());
            s.previous_legato_threshold = std::nullopt;
        }

        return triggers;
    }




    static Voice<Trigger> trigger_pulse(const Phase& cursor, State& s, const Params& p) {
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
                , crossing_direction == ThresholdDirection::forward ? legato_fraction : 1.0 - legato_fraction
                , 0
                , 0
                , static_cast<std::size_t>(p.legato > 1.0)
                , crossing_direction
            };
        } else {
            triggers.append(Trigger::pulse_off(pulse_on.get_id()));
        }
    }

    static void flip_legato_thresholds(const ThresholdDirection& new_direction, State& s, const Params& p) {
        if (s.previous_legato_threshold) {
            s.previous_legato_threshold->expected_direction = new_direction;
        }

        if (s.current_legato_threshold) {
            s.current_legato_threshold->expected_direction = new_direction;
        }
    }


    static bool detect_jump_to_threshold(const Phase& cursor, const State& s, const Params& p) {
        // First value received
        if (!s.previous_cursor) {
            return close_to_threshold(cursor);
        }

        return close_to_threshold(cursor) && s.previous_cursor->distance_to(cursor) > JUMP_DETECTION_THRESHOLD;
    }


    static bool crosses_threshold(const Phase& cursor, const State& s) {
        if (!s.previous_cursor)
            return false;

        return PhasePulsatorStrategies::is_wrap_around(*s.previous_cursor, cursor);
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

class MultiThresholdStrategy {
public:
    static constexpr double JUMP_PROXIMITY_THRESHOLD = 1e-3;

    using State = PhasePulsatorState;
    using Params = PhasePulsatorParameters;

    SingleThresholdStrategy() = delete;


    static Voice<Trigger> process(const Phase& cursor, State& s, const Params& p) {
        Voice<Trigger> triggers;

        auto threshold = threshold_close_to(c);


        if (is_jump_to_threshold(threshold, s)) {
            triggers.extend(handle_continuous_jump(*threshold));
        } else if (auto segment = segment_changed(cursor, s)) {
            if (is_threshold_crossing(*segment, s)) {
                triggers.extend(handle_threshold_crossing(*segment));
            } else {
                triggers.extend(handle_discontinuous_jump(*segment));
            }
        }

        triggers.extend(process_legato_thresholds(cursor));

        s.previous_cursor = cursor;
        return triggers;
    }


    static Voice<Trigger> on_activate(State& s, const Params& p);


    static Voice<Trigger> handle_legato_change(State& s, const Params& p);

private:
    static bool is_jump_to_threshold(const std::optional<ThresholdIndex> threshold, const State& s) {
        return threshold && !is_adjacent(*threshold, s);
    }


    static bool is_threshold_crossing(const DurationIndex segment, const State& s) {
        return is_adjacent(segment);
    }


    static std::optional<SegmentIndex> detect_jump_to_segment(cursor, const State& s) {}


    static std::optional<ThresholdIndex> detect_threshold_crossing(cursor, const State& s);

    // State Mutators
    static Voice<Trigger> handle_continuous_jump(ThresholdIndex threshold, State& s, const Params& p);
    static Voice<Trigger> handle_discontinuous_jump(SegmentIndex segment, State& s, const Params& p);


    static Voice<Trigger> handle_threshold_crossing(SegmentIndex segment, State& s, const Params& p) {
        auto threshold = threshold_crossed(s.current_segment, segment);
        if (is_same_as_last(threshold)) {
            flip_legato_thresholds(segment, threshold);
            return {};

            // TODO .. (and don't forget to handle pauses)
        }
    }


    static process_legato_thresholds(const Phase& cursor, State& s);
    void flip_legato_thresholds(SegmentIndex new_segment
                                , ThresholdIndex threshold_crossed
                                , State& s
                                , const Params& p);

    static std::optional<ThresholdIndex> threshold_close_to(const Phase& cursor, const State& s);
    static bool is_adjacent(ThresholdIndex threshold, const State& s);
    static bool is_same_as_last(ThresholdIndex threshold, const State& s);

    static std::optional<DurationIndex> segment_changed(const Phase& cursor);
    static DurationIndex segment_of(const Phase& cursor, const Params& p);
    static bool is_adjacent(SegmentIndex);
};


// ==============================================================================================

class TempPhasePulsator {
public:
    Voice<Trigger> process(const Phase& cursor) {
        assert_invariants();

        Voice<Trigger> triggers;

        if (duration_change_applicable()) {
            triggers.extend(handle_duration_change());
        }

        if (m_legato.changed()) {
            triggers.extend(handle_legato_change());
        }

        triggers.extend(process_cursor(cursor));
        return triggers;
    }


    void set_legato(double legato) {
        assert(legato >= 0.0);
        m_parameters.new_legato = legato;
    }


    void set_durations(Vec<double> durations) {
        assert(!durations.empty());
        m_parameters.new_durations = durations;
    }

private:
    Voice<Trigger> handle_duration_change() {
        assert(m_parameters.new_durations);
        assert(!m_parameters.new_durations->empty());

        auto new_durations = *m_parameters.new_durations;

        auto new_size = new_durations.size();
        auto old_size = m_parameters.durations.size();

        // negative durations d correspond to pauses with duration abs(d)
        m_parameters.is_pause = new_durations.as_type<bool>([](double d) { return d <= 0.0; });
        m_parameters.durations = new_durations.map([](double d) { return std::abs(d); });

        // Normalize to 1.0
        auto sum = m_parameters.durations.sum();
        assert(sum > 0.0);
        m_parameters.durations.map([sum](double d) { return d / sum; });

        m_parameters.new_durations = std::nullopt;

        if (new_size == 1 && old_size != 1) {
            return SingleThresholdStrategy::on_activate(m_state, m_parameters);
        } else if (new_size > 1 && old_size == 1) {
            return MultiThresholdStrategy::on_activate(m_state, m_parameters);
        }
        return {}; // Change from N > 1 to M > 1
    };


    Voice<Trigger> handle_legato_change() {
        assert(m_parameters.new_legato);

        m_parameters.legato = *m_parameters.new_legato;
        m_parameters.new_legato = std::nullopt;

        if (m_durations.size() == 1) {
            return SingleThresholdStrategy::handle_legato_change(m_state, m_parameters);
        } else {
            return MultiThresholdStrategy::handle_legato_change(m_state, m_parameters);
        }
    }


    Voice<Trigger> process_cursor(const Phase& cursor) {
        if (m_durations.size() == 1) {
            return SingleThresholdStrategy::process(cursor, m_state, m_parameters);
        } else {
            return MultiThresholdStrategy::process(cursor, m_state, m_parameters);
        }
    }


    bool legato_changed() const { return m_parameters.new_legato; }


    bool duration_change_applicable(const Phase& cursor) const {
        return m_parameters.m_new_durations
               && (m_parameters.apply_duration_change_immediately
                   || (PhasePulsatorStrategies::is_wrap_around(m_state.previous_cursor, cursor) ||
                       PhasePulsatorStrategies::is_jump_to_threshold(0, cursor, m_state, m_parameters))
               );
    }

    void assert_invariants() const {
        // previous legato threshold should never exist if current legato threshold doesn't
        assert(!m_state.previous_legato_threshold || m_state.current_legato_threshold);
    }

private:
    PhasePulsatorState m_state;
    PhasePulsatorParameters m_parameters;
};
}

#endif // SERIALIST_PHASE_PULSATOR_H
