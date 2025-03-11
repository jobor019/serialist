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

    bool contains(double position);
};

struct LegatoThreshold {
    std::size_t trigger_id;
    double legato_value;

    double threshold_position;

    ThresholdIndex pulse_on_threshold;
    ThresholdIndex associated_threshold; // Different if legato > 1.0

    // Only relevant for N=1
    std::size_t num_remaining_passes = 0;
    // Only relevant for N=1, but should be handled in N>1 too since
    //   existing legato point might change from N>1 to N=1 on change of dur
    ThresholdDirection expected_direction;


    // Return true if repositioning would move the threshold past
    //   the cursor. Also, implementation is probably different for
    //   N=1 and N>1
    bool reposition(double new_legato_value, const Phase& cursor);

    bool flip(ThresholdIndex last_threshold);
};


struct PhasePulsatorState {
    std::optional<Phase> previous_cursor = std::nullopt;

    std::optional<ThresholdIndex> last_threshold = std::nullopt;
    std::optional<ThresholdIndex> expected_next_threshold = std::nullopt;
    std::optional<DurationIndex> current_segment = std::nullopt;

    std::optional<LegatoThreshold> previous_legato_threshold = std::nullopt;
    std::optional<LegatoThreshold> current_legato_threshold = std::nullopt;
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


    static Voice<Trigger> on_activate(State& s, const Params& p);
    static Voice<Trigger> handle_legato_change(double old_legato_value, State& s, const Params& p);

private:

    static Voice<Trigger> continuous_jump(const Phase& cursor, State& s, const Params& p);
    static Voice<Trigger> handle_threshold_crossing(const Phase& cursor, State& s, const Params& p);
    static Voice<Trigger> process_legato_thresholds(const Phase& cursor, State& s, const Params& p);


    static bool detect_jump_to_threshold(const Phase& cursor, const State& s, const Params& p);
    static bool crosses_threshold(const Phase& cursor, const State& s, const Params& p);

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


    static Voice<Trigger> handle_legato_change(double old_legato_value, State& s, const Params& p);


private:
    static bool is_jump_to_threshold(const std::optional<ThresholdIndex> threshold, const State& s) {
        return threshold && !is_adjacent(*threshold, s);
    }


    static bool is_threshold_crossing(const DurationIndex segment, const State& s) {
        return is_adjacent(segment);
    }


    static std::optional<SegmentIndex> detect_jump_to_segment(cursor, const State& s) {

    }


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


    void set_legato(double legato) { m_parameters.new_legato = legato; }

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
        m_parameters.durations.map([sum](double d){ return d / sum; });

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

        double old_legato_value = m_parameters.legato;
        m_parameters.legato = *m_parameters.new_legato;
        m_parameters.new_legato = std::nullopt;

        if (old_legato_value > m_parameters.m_legato) {
            if (m_durations.size() == 1) {
                return SingleThresholdStrategy::handle_legato_change(old_legato_value, m_state, m_parameters);
            } else {
                return MultiThresholdStrategy::handle_legato_change(old_legato_value, m_state, m_parameters);
            }
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

private:
    PhasePulsatorState m_state;
    PhasePulsatorParameters m_parameters;
};

}

#endif // SERIALIST_PHASE_PULSATOR_H