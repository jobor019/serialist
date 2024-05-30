
#ifndef SERIALISTLOOPER_PULSATOR_H
#define SERIALISTLOOPER_PULSATOR_H

#include "core/algo/temporal/pulse.h"
#include "core/algo/temporal/trigger.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "core/algo/temporal/time_point.h"
#include "core/utility/stateful.h"
#include "core/algo/temporal/time_point_generators.h"
#include "variable.h"
#include "sequence.h"
#include <map>

enum class PulsatorMode {
    free_periodic
    , transport_locked
    , free_triggered
    , thru
};

struct PulsatorParameters {
    static constexpr inline double DEFAULT_DURATION = 1.0;
    static const inline DomainType DEFAULT_DURATION_TYPE = DomainType::ticks;
    static constexpr inline double DEFAULT_OFFSET = 0.0;
    static const inline DomainType DEFAULT_OFFSET_TYPE = DomainType::ticks;
    static constexpr inline double DEFAULT_LEGATO_AMOUNT = 1.0;
    static const inline bool DEFAULT_SNH = false;


    WithChangeFlag<DomainDuration> duration{DomainDuration{DEFAULT_DURATION, DEFAULT_DURATION_TYPE}};
    WithChangeFlag<DomainDuration> offset{DomainDuration{DEFAULT_OFFSET, DEFAULT_OFFSET_TYPE}};
    WithChangeFlag<double> legato_amount{DEFAULT_LEGATO_AMOUNT};

    bool sample_and_hold = DEFAULT_SNH;
    Voice<Trigger> triggers;

    static void clear_flags(PulsatorParameters& p) {
        p.duration.clear_flag();
        p.offset.clear_flag();
        p.legato_amount.clear_flag();
    }
};


// ==============================================================================================

struct PulsatorState {

    PulsatorState() = default;

    ~PulsatorState() = default;

    // Copy ctor: do not move pulses, as this will generate redundant pulse_offs
    PulsatorState(const PulsatorState& other)
            : next_trigger_time(other.next_trigger_time)
              , current_trigger_time(other.current_trigger_time) {}

    // Copy assignment operator: do not move pulses, as this will generate redundant pulse_offs
    PulsatorState& operator=(const PulsatorState& other) {
        if (this == &other) return *this;
        next_trigger_time = other.next_trigger_time;
        current_trigger_time = other.current_trigger_time;
        return *this;
    }

    PulsatorState(PulsatorState&&) noexcept = default;

    PulsatorState& operator=(PulsatorState&&) noexcept = default;

    Pulses pulses;
    std::optional<DomainTimePoint> next_trigger_time = std::nullopt;
    std::optional<DomainTimePoint> current_trigger_time = std::nullopt;

    std::optional<TimePoint> last_callback_time = std::nullopt;

//    Voice<Trigger> last_output;

};


// ==============================================================================================

class PulsatorStrategy {
public:
    PulsatorStrategy() = default;

    virtual ~PulsatorStrategy() = default;

    PulsatorStrategy(const PulsatorStrategy&) = default;

    PulsatorStrategy& operator=(const PulsatorStrategy&) = default;

    PulsatorStrategy(PulsatorStrategy&&) noexcept = default;

    PulsatorStrategy& operator=(PulsatorStrategy&&) noexcept = default;

    virtual Voice<Trigger> process(const TimePoint& t, PulsatorState& s, const PulsatorParameters& p) const = 0;

    /**
     * @brief Called once when switching from another strategy
     */
    virtual Voice<Trigger> activate(const TimePoint& t, PulsatorState& s, const PulsatorParameters& p) const {
        (void) t;
        (void) s;
        (void) p;
        return {};
    }


    virtual Voice<Trigger> flush(PulsatorState& s) const {
        return s.pulses.flush();
    }


    /**
     * @brief Called when the node is disabled (and only then, not when transport is stopped)
     */
    virtual Voice<Trigger> clear(PulsatorState& s) const {
        s.next_trigger_time = std::nullopt;
        s.last_callback_time = std::nullopt;
        s.current_trigger_time = std::nullopt;
        return flush(s);
    }


    virtual Voice<Trigger> on_time_skip(const TimePoint& t, PulsatorState& s, const PulsatorParameters& p) const {
        (void) p;
        s.next_trigger_time = std::nullopt;
        s.current_trigger_time = std::nullopt;
        s.last_callback_time = t;
        return flush(s);
    }


    static DomainTimePoint as_dtp(const TimePoint& time, const PulsatorParameters& p) {
        return DomainTimePoint::from_time_point(time, p.duration->get_type());
    }


    static Voice<Trigger> drain_endless_and_non_matching(PulsatorState& s, const PulsatorParameters& p) {
        Voice<Trigger> output = s.pulses.drain_endless_as_triggers();
        output.extend(drain_non_matching(s, p));
        return output;
    }

    static Voice<Trigger> drain_non_matching(PulsatorState& s, const PulsatorParameters& p) {
        return s.pulses.drain_non_matching_as_triggers(p.duration->get_type());
    }


    static void clear_scheduled_non_matching(PulsatorState& s, DomainType type) {
        if (s.next_trigger_time && s.next_trigger_time->get_type() != type) {
            s.next_trigger_time = std::nullopt;
        }

        if (s.current_trigger_time && s.current_trigger_time->get_type() != type) {
            s.current_trigger_time = std::nullopt;
        }
    }

    static void assert_domain_type_correctness(PulsatorState& s, const PulsatorParameters& p) {
        assert(!s.current_trigger_time || s.current_trigger_time->get_type() == p.duration->get_type());
        assert(!s.next_trigger_time || s.next_trigger_time->get_type() == p.duration->get_type());
        assert(s.pulses.vec().all([](const Pulse& pulse) { return pulse.has_pulse_off(); }));
        assert(s.pulses.vec().all([&p](const Pulse& pulse) {
            return pulse.get_type() == p.duration->get_type();
        }));
    }
};


// ==============================================================================================

class Thru : public PulsatorStrategy {
public:

    Voice<Trigger> activate(const TimePoint&, PulsatorState& s, const PulsatorParameters&) const override {
        s.next_trigger_time = std::nullopt;
        s.current_trigger_time = std::nullopt;
        return s.pulses.flush();
    }

    Voice<Trigger> process(const TimePoint& t, PulsatorState& s, const PulsatorParameters& p) const override {

        // Note: We do not need to assert domain type correctness here. If duration changes, this may lead to different
        // DomainType pulses in s.pulses. This is not a problem for this strategy, but all other strategies should
        // handle such a state on activate()

        Voice<Trigger> output;

        for (const auto& trigger: p.triggers) {
            if (trigger.is(Trigger::Type::pulse_on)) {
                auto dtp = as_dtp(t, p);
                output.append(s.pulses.new_pulse(dtp, std::nullopt, trigger.get_id()));
                s.current_trigger_time = dtp;
            } else if (trigger.is(Trigger::Type::pulse_off)) {
                output.extend(s.pulses.drain_by_id_as_triggers(trigger.get_id()));
            }
        }

        if (s.pulses.empty()) {
            // clear current_trigger_time if last pulse off has been processed
            s.current_trigger_time = std::nullopt;
        }

        return output;
    }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class FreeTriggered : public PulsatorStrategy {
public:

    Voice<Trigger> activate(const TimePoint&, PulsatorState& s, const PulsatorParameters&) const override {
        s.next_trigger_time = std::nullopt;
        s.current_trigger_time = std::nullopt;
        return s.pulses.flush();
    }

    Voice<Trigger> process(const TimePoint& t, PulsatorState& s, const PulsatorParameters& p) const override {
        Voice<Trigger> output;

        if (!p.sample_and_hold && (p.duration.changed() || p.legato_amount.changed())) {
            output.extend(reschedule(s, p));
        }

        assert_domain_type_correctness(s, p);

        output.extend(s.pulses.drain_elapsed_as_triggers(t, true));


        if (auto trigger_idx = p.triggers.index([](const Trigger& trigger) {
            return trigger.is(Trigger::Type::pulse_on);
        })) {
            output.append(schedule_new(as_dtp(t, p), s, p, p.triggers[*trigger_idx].get_id()));
        }

        if (s.pulses.empty()) {
            // clear current_trigger_time if last pulse off has been processed
            s.current_trigger_time = std::nullopt;
        }

        return output;
    }

private:
    static DomainDuration duration(const PulsatorParameters& p) {
        return *p.duration * *p.legato_amount;
    }

    static Trigger schedule_new(const DomainTimePoint& dtp
                                , PulsatorState& s
                                , const PulsatorParameters& p
                                , std::size_t trigger_id) {
        auto off = dtp + duration(p);
        s.current_trigger_time = dtp;
        return s.pulses.new_pulse(dtp, off, trigger_id);
    }

    static Voice<Trigger> reschedule(PulsatorState& s, const PulsatorParameters& p) {
        Voice<Trigger> output;

        if (p.duration.changed()) {
            output.extend(drain_non_matching(s, p));
        }

        s.pulses.try_set_durations(duration(p));

        return output;
    }

};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class FreePeriodicGenerator : public PulsatorStrategy {
public:

    Voice<Trigger> activate(const TimePoint& t, PulsatorState& s, const PulsatorParameters& p) const override {
        Voice<Trigger> output = drain_endless_and_non_matching(s, p);
        clear_scheduled_non_matching(s, p.duration->get_type());

        // From this point, s.pulses, s.next_trigger_time and s.current_trigger_time will
        // either be empty/nullopt or same type as duration

        if (s.current_trigger_time && !s.next_trigger_time) {
            // no next trigger scheduled but ongoing pulse exists
            s.next_trigger_time = next_or_now(*s.current_trigger_time, t, p);

        } else if (!s.next_trigger_time.has_value() && !s.pulses.empty()) {
            if (auto last_pulse_end = s.pulses.last_of_type(p.duration->get_type())) {
                // no next trigger scheduled but ongoing pulses of the correct type exist in s.pulses
                s.next_trigger_time = last_pulse_end->get_pulse_off_time();
                s.current_trigger_time = last_pulse_end->get_trigger_time();
            }
        }

        if (s.next_trigger_time && !s.next_trigger_time) {
            // if only next trigger exists, "reconstruct" s.current_trigger_time from s.next_trigger_time
            s.current_trigger_time = *s.next_trigger_time - duration(p);
        }

        assert_invariants(s, p);

        // if no valid triggers exist, these will be scheduled in next process() call

        return output;
    }


    Voice<Trigger> process(const TimePoint& t, PulsatorState& s, const PulsatorParameters& p) const override {
        Voice<Trigger> output;

        if (!p.sample_and_hold && (p.duration.changed() || p.legato_amount.changed())) {
            output.extend(reschedule(t, s, p));
        }

        assert_invariants(s, p);

        output.extend(s.pulses.drain_elapsed_as_triggers(t, true));

        if (!s.next_trigger_time.has_value()) {
            output.append(schedule_new(as_dtp(t, p), s, p));

        } else if (s.next_trigger_time->elapsed(t)) {
            output.append(schedule_new(*s.next_trigger_time, s, p));
        }

        return output;
    }


private:
    static DomainDuration duration(const PulsatorParameters& p) {
        return *p.duration * *p.legato_amount;
    }


    static DomainTimePoint next(const DomainTimePoint& dtp, const PulsatorParameters& p) {
        return dtp + *p.duration;
    }


    static DomainTimePoint next_or_now(const DomainTimePoint& dtp
                                       , const TimePoint& now
                                       , const PulsatorParameters& p) {
        return DomainTimePoint::max(as_dtp(now, p), next(dtp, p));
    }


    static Trigger schedule_new(const DomainTimePoint& dtp, PulsatorState& s, const PulsatorParameters& p) {
        auto off = dtp + duration(p);
        s.next_trigger_time = dtp + *p.duration;
        s.current_trigger_time = dtp;
        return s.pulses.new_pulse(dtp, off);
    }


    static Voice<Trigger> reschedule(const TimePoint& t, PulsatorState& s, const PulsatorParameters& p) {
        Voice<Trigger> output;

        // legato OR duration changed
        s.pulses.try_set_durations(duration(p));

        if (s.current_trigger_time.has_value()) { // => invariant: s.next_trigger_time.has_value()
            if (s.current_trigger_time->get_type() != p.duration->get_type()) {
                // Duration type changed: adjust to new type and flush non-matching
                s.current_trigger_time = s.current_trigger_time->as_type(p.duration->get_type(), t);
                output.extend(drain_non_matching(s, p));
            }

            if (p.duration.changed()) {
                // Duration value (or type) changed: reschedule next
                s.next_trigger_time = next_or_now(*s.current_trigger_time, t, p);
            }
        }

        return output;
    }

    static void assert_invariants(PulsatorState& s, const PulsatorParameters& p) {
        assert(s.current_trigger_time.has_value() == s.next_trigger_time.has_value());
        assert_domain_type_correctness(s, p);
    }
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class TransportLockedGenerator : public PulsatorStrategy {
public:
    Voice<Trigger> activate(const TimePoint& t, PulsatorState& s, const PulsatorParameters& p) const override {
        auto output = drain_endless_and_non_matching(s, p);
        clear_scheduled_non_matching(s, p.duration->get_type());

        // From this point, s.pulses, s.next_trigger_time and s.current_trigger_time will
        // either be empty/nullopt or same type as duration

        if (s.next_trigger_time.has_value()) {
            // scheduled trigger exists -- adjust to grid
            s.next_trigger_time = TransportLocked::adjusted(*s.next_trigger_time, t, *p.duration, *p.offset);

        } else if (s.current_trigger_time && !s.next_trigger_time) {
            // no trigger scheduled but ongoing pulse exists
            s.next_trigger_time = TransportLocked::next_from_either(*s.current_trigger_time, t, *p.duration, *p.offset);

        } else if (!s.next_trigger_time.has_value() && !s.pulses.empty()) {
            if (auto last_pulse_end = s.pulses.last_of_type(p.duration->get_type())) {
                if (last_pulse_end->has_pulse_off()) {
                    // no trigger scheduled but ongoing pulses of the correct type exist in s.pulses
                    s.next_trigger_time = TransportLocked::next_from_either(*last_pulse_end->get_pulse_off_time(), t
                                                                            , *p.duration, *p.offset);
                }
            }
        }

        if (s.next_trigger_time) {
            // if only next trigger exists, "reconstruct" s.current_trigger_time from grid-aligned s.next_trigger_time
            s.current_trigger_time = *s.next_trigger_time - *p.duration;
        }

        assert_invariants(s, p);

        return output;
    }


    Voice<Trigger> process(const TimePoint& t, PulsatorState& s, const PulsatorParameters& p) const override {
        Voice<Trigger> output;

        if (!p.sample_and_hold && (p.duration.changed() || p.legato_amount.changed() || p.offset.changed())) {
            output.extend(reschedule_pulse_offs(t, s, p));
            reschedule_next_trigger(t, s, p);
        } else if (!p.sample_and_hold && p.legato_amount.changed()) {
            output.extend(reschedule_pulse_offs(t, s, p));
        }

        assert_invariants(s, p);

        output.extend(s.pulses.drain_elapsed_as_triggers(t, true));

        if (!s.next_trigger_time.has_value()) {
            schedule_first_trigger(t, s, p);


        } else if (s.next_trigger_time->elapsed(t)) {
            output.append(schedule_new(*s.next_trigger_time, t, s, p));
        }

        // At the end of each process call, s.next_trigger_time and s.current_trigger_time are always set.
        // This is not an invariant for the class, as it may be unset elsewhere (time skip, enabled/disabled, etc.)
        assert(s.next_trigger_time.has_value());
        assert(s.current_trigger_time.has_value());

        return output;

    }

private:
    /** @throws TimeDomainError if dtp is not compatible with p.duration */
    static DomainTimePoint next(const DomainTimePoint& dtp
                                , const TimePoint& t
                                , const PulsatorParameters& p) {
        return TransportLocked::next(dtp, *p.duration, *p.offset, t.get_meter(), false);
    }

    static void schedule_first_trigger(const TimePoint& t, PulsatorState& s, const PulsatorParameters& p) {
        s.current_trigger_time = as_dtp(t, p);
        s.next_trigger_time = TransportLocked::next(t, *p.duration, *p.offset, true);
    }


    static Trigger schedule_new(const DomainTimePoint& dtp
                                , const TimePoint& t
                                , PulsatorState& s
                                , const PulsatorParameters& p) {
        s.current_trigger_time = s.next_trigger_time.value_or(as_dtp(t, p));
        s.next_trigger_time = next(dtp, t, p);
        auto dur = *s.next_trigger_time - *s.current_trigger_time;
        auto off = *s.current_trigger_time + dur * *p.legato_amount;
        return s.pulses.new_pulse(*s.current_trigger_time, off);
    }


    static void reschedule_next_trigger(const TimePoint& t, PulsatorState& s, const PulsatorParameters& p) {
        if (s.current_trigger_time) { // => Invariant: s.next_trigger_time.has_value()
            if (!p.duration->supports(*s.current_trigger_time)) {
                s.current_trigger_time = s.current_trigger_time->as_type(p.duration->get_type(), t);
            }
            s.next_trigger_time = TransportLocked::next_from_either(*s.current_trigger_time, t, *p.duration, *p.offset);

        }
        // if neither current_trigger_time nor next_trigger_time are defined (first call / time skip), there's nothing to reschedule
    }

    static Voice<Trigger> reschedule_pulse_offs(const TimePoint& t, PulsatorState& s, const PulsatorParameters& p) {
        auto output = drain_non_matching(s, p);

        for (auto& pulse: s.pulses.vec_mut()) {
            auto on = pulse.get_trigger_time();
            auto off = next(on, t, p);
            pulse.try_set_duration((on - off) * *p.legato_amount);
        }

        return output;
    }


    static void assert_invariants(PulsatorState& s, const PulsatorParameters& p) {
        assert(s.current_trigger_time.has_value() == s.next_trigger_time.has_value());
        assert_domain_type_correctness(s, p);
    }
};


// ==============================================================================================

class Pulsator : public Flushable<Trigger> {
public:
    static const inline PulsatorMode DEFAULT_MODE = PulsatorMode::transport_locked;

    Pulsator() = default;

    ~Pulsator() override = default;

    // No need to copy m_strategies as they are completely static
    Pulsator(const Pulsator& other)
            : m_params(other.m_params)
              , m_state(other.m_state)
              , m_mode(other.m_mode) {}


    // No need to copy m_strategies as they are completely static
    Pulsator& operator=(const Pulsator& other) {
        if (this == &other) return *this;
        m_params = other.m_params;
        m_state = other.m_state;
        m_mode = other.m_mode;
        return *this;
    }

    Pulsator(Pulsator&&) noexcept = default;

    Pulsator& operator=(Pulsator&&) noexcept = default;

    Voice<Trigger> process(const TimePoint& time) {
        Voice<Trigger> output;
        if (m_mode.changed()) {
            output.extend(active_strategy().activate(time, m_state, m_params));
        }

        output.extend(active_strategy().process(time, m_state, m_params));

        m_state.last_callback_time = time;
        m_params.triggers.clear(); // responsibility of main class, params are const to the strategy

        PulsatorParameters::clear_flags(m_params);
        m_mode.clear_flag();

        return output;
    }

    /**
     * @brief Called only when pulsator is disabled (not when transport is paused)
     */
    Voice<Trigger> clear() { return active_strategy().clear(m_state); }

    Voice<Trigger> flush() override { return active_strategy().flush(m_state); }

    Voice<Trigger> handle_time_skip(const TimePoint& new_time) {
        return active_strategy().on_time_skip(new_time, m_state, m_params);
    }

    void set_duration(const DomainDuration& duration) { m_params.duration = duration; }

    void set_offset(const DomainDuration& offset) { m_params.offset = offset; }

    void set_legato_amount(double legato_amount) { m_params.legato_amount = legato_amount; }

    void set_sample_and_hold(bool sample_and_hold) { m_params.sample_and_hold = sample_and_hold; }

    void set_triggers(const Voice<Trigger>& t) { m_params.triggers = t; }

    void set_mode(PulsatorMode mode) { m_mode = mode; }


private:
    PulsatorStrategy& active_strategy() {
        return *m_strategies[*m_mode];
    }


    static std::map<PulsatorMode, std::unique_ptr<PulsatorStrategy>> make_strategies() {
        std::map<PulsatorMode, std::unique_ptr<PulsatorStrategy>> map;
        map.insert(std::make_pair(PulsatorMode::transport_locked, std::make_unique<TransportLockedGenerator>()));
        map.insert(std::make_pair(PulsatorMode::free_periodic, std::make_unique<FreePeriodicGenerator>()));
        map.insert(std::make_pair(PulsatorMode::free_triggered, std::make_unique<FreeTriggered>()));
        map.insert(std::make_pair(PulsatorMode::thru, std::make_unique<Thru>()));
        return map;
    }

    std::map<PulsatorMode, std::unique_ptr<PulsatorStrategy>> m_strategies = make_strategies();

    PulsatorParameters m_params;
    PulsatorState m_state;
    WithChangeFlag<PulsatorMode> m_mode = DEFAULT_MODE;


};

// ==============================================================================================

class PulsatorNode : public NodeBase<Trigger> {
public:

    // TODO
    class PulsatorKeys {
    public:
        PulsatorKeys() = delete;

        static const inline std::string MODE = "mode";
        static const inline std::string TRIGGER = "trigger";
        static const inline std::string DURATION = "duration";
        static const inline std::string DURATION_TYPE = "duration_type";
        static const inline std::string OFFSET = "offset";
        static const inline std::string OFFSET_TYPE = "offset_type";
        static const inline std::string LEGATO_AMOUNT = "legato_amount";
        static const inline std::string SAMPLE_AND_HOLD = "sample_and_hold";

        static const inline std::string CLASS_NAME = "pulsator";
    };

    PulsatorNode(const std::string& id
                 , ParameterHandler& parent
                 , Node<Facet>* mode = nullptr
                 , Node<Trigger>* trigger = nullptr
                 , Node<Facet>* duration = nullptr
                 , Node<Facet>* duration_type = nullptr
                 , Node<Facet>* offset = nullptr
                 , Node<Facet>* offset_type = nullptr
                 , Node<Facet>* legato_amount = nullptr
                 , Node<Facet>* sample_and_hold = nullptr
                 , Node<Facet>* enabled = nullptr
                 , Node<Facet>* num_voices = nullptr)
            : NodeBase<Trigger>(id, parent, enabled, num_voices, PulsatorKeys::CLASS_NAME)
              , m_mode(add_socket(PulsatorKeys::MODE, mode))
              , m_trigger(add_socket(PulsatorKeys::TRIGGER, trigger))
              , m_duration(add_socket(PulsatorKeys::DURATION, duration))
              , m_duration_type(add_socket(PulsatorKeys::DURATION_TYPE, duration_type))
              , m_offset(add_socket(PulsatorKeys::OFFSET, offset))
              , m_offset_type(add_socket(PulsatorKeys::OFFSET_TYPE, offset_type))
              , m_legato_amount(add_socket(PulsatorKeys::LEGATO_AMOUNT, legato_amount))
              , m_sample_and_hold(add_socket(PulsatorKeys::SAMPLE_AND_HOLD, sample_and_hold)) {}

    Voices<Trigger> process() override {
        auto t = pop_time();
        if (!t) // process has already been called this cycle
            return m_current_value;

        if (!update_enabled_state(*t)) {
            return m_current_value; // already flushed/updated by update_enabled_state() if needed
        }

        auto num_voices = get_voice_count();
        Voices<Trigger> output = Voices<Trigger>::zeros(num_voices);

        bool resized = false;
        if (auto flushed = update_size(num_voices)) {
            if (!flushed->is_empty_like()) {
                // from this point on, size of output may be different from num_voices,
                //   but this is the only point where resizing should be allowed
                output.merge_uneven(*flushed, true);
            }
            resized = true;
        }

        if (auto flushed = handle_transport_state(*t)) {
            output.merge_uneven(*flushed, false);
        }


        update_parameters(num_voices, resized);

        for (std::size_t i = 0; i < m_pulsators.size(); ++i) {
            output[i].extend(m_pulsators[i].process(*t));
        }

        m_current_value = std::move(output);
        return m_current_value;
    }

private:
    std::size_t get_voice_count() {
        return voice_count(m_trigger.voice_count()
                           , m_duration.voice_count()
                           , m_offset.voice_count()
                           , m_legato_amount.voice_count());
    }

//    /**
//     * @brief Returns true if the node is enabled. If it was just disabled, flushed pulsators to m_current_value.
//     */
//    bool update_enabled_state(const TimePoint& t) {
//        bool enabled = is_enabled(t);
//        if (!enabled) {
//            if (m_previous_enable_state) {
//                // if it was disabled this cycle, stop all pulsators
//                m_current_value = stop();
//            } else if (!m_current_value.is_empty_like()) {
//                // if it was disabled the previous cycle, clear all output
//                m_current_value = Voices<Trigger>::empty_like();
//            } // if it was disabled any earlier cycle: output should already be empty
//
//        } else if (!m_previous_enable_state) {
//            // if it was enabled this cycle, start
//            start(t);
//        }
//
//        m_previous_enable_state = enabled;
//
//        return enabled;
//    }

    bool update_enabled_state(const TimePoint& t) {
        bool enabled = is_enabled(t);
        switch (m_enabled_gate.update(enabled)) {
            case EnabledState::enabled:
                return true;
            case EnabledState::enabled_this_cycle:
                start(t);
                return true;
            case EnabledState::disabled_this_cycle:
                m_current_value = stop();
                return false;
            case EnabledState::disabled_previous_cycle:
                m_current_value = Voices<Trigger>::empty_like();
                return false;
            case EnabledState::disabled:
                return false;
        }
    }

    /** @returns flushed triggers and flag indicating whether to */
    std::optional<Voices<Trigger>> handle_transport_state(const TimePoint& t) {
        auto events = m_time_event_gate.poll(t);

        if (events.contains(TimeEvent::time_skip)) {
            auto num_voices = m_pulsators.size();
            auto flushed = Voices<Trigger>::zeros(num_voices);
            for (std::size_t i = 0; i < num_voices; ++i) {
                flushed[i] = m_pulsators[i].handle_time_skip(t);
            }
            return flushed;
        }

        return std::nullopt;
    }

    /**
     * @return flushed triggers (Voices<Trigger>) if num_voices has changed, std::nullopt otherwise
     *         note that the flushed triggers will have the same size as the previous num_voices, hence
     *         merge_uneven(.., true) is required
     */
    std::optional<Voices<Trigger>> update_size(std::size_t num_voices) {
        if (num_voices != m_pulsators.size()) {
//            auto initial_size = m_pulsators.size();
            auto flushed = m_pulsators.resize(num_voices);
//            start(t, initial_size, num_voices);
            return flushed;
        }

        return std::nullopt;
    }


    void start(const TimePoint&) {
        m_time_event_gate.reset();
//        start(t, 0, m_pulsators.size());
    }


    void update_parameters(std::size_t num_voices, bool size_has_changed) {
        if (size_has_changed || m_mode.has_changed()) {
            auto mode = m_offset.process().first_or(Pulsator::DEFAULT_MODE);
            m_pulsators.set(&Pulsator::set_mode, mode);
        }

        auto triggers = m_trigger.process().adapted_to(num_voices);
        m_pulsators.set(&Pulsator::set_triggers, std::move(triggers.vec_mut()));

        if (size_has_changed || m_duration.has_changed() || m_duration_type.has_changed()) {
            auto duration = m_duration.process().adapted_to(num_voices).firsts_or(PulsatorParameters::DEFAULT_DURATION);
            auto duration_type = m_duration_type.process().first_or(PulsatorParameters::DEFAULT_DURATION_TYPE);

            auto durations = Vec<DomainDuration>::allocated(num_voices);
            for (const auto& d: duration) {
                durations.append(DomainDuration(d, duration_type));
            }

            m_pulsators.set(&Pulsator::set_duration, std::move(durations));
        }

        if (size_has_changed || m_offset.has_changed() || m_offset_type.has_changed()) {
            auto offset = m_offset.process().adapted_to(num_voices).firsts_or(PulsatorParameters::DEFAULT_OFFSET);
            auto offset_type = m_offset_type.process().first_or(PulsatorParameters::DEFAULT_OFFSET_TYPE);

            auto offsets = Vec<DomainDuration>::allocated(num_voices);
            for (const auto& o: offset) {
                offsets.append(DomainDuration(o, offset_type));
            }

            m_pulsators.set(&Pulsator::set_offset, std::move(offsets));
        }

        if (size_has_changed || m_legato_amount.has_changed()) {
            auto legato = m_legato_amount.process()
                    .adapted_to(num_voices)
                    .firsts_or(PulsatorParameters::DEFAULT_LEGATO_AMOUNT);
            m_pulsators.set(&Pulsator::set_legato_amount, std::move(legato));
        }

        if (size_has_changed || m_sample_and_hold.has_changed()) {
            auto sample_and_hold = m_sample_and_hold.process().first_or(PulsatorParameters::DEFAULT_SNH);
            m_pulsators.set(&Pulsator::set_sample_and_hold, sample_and_hold);
        }
    }


    Voices<Trigger> stop() {
        m_time_event_gate.reset();
        auto flushed = Voices<Trigger>::zeros(m_pulsators.size());

        for (std::size_t i = 0; i < m_pulsators.size(); ++i) {
            flushed[i] = m_pulsators.get_objects()[i].clear();
        }

        return flushed;
    }


    MultiVoiced<Pulsator, Trigger> m_pulsators;

    Voices<Trigger> m_current_value = Voices<Trigger>::empty_like();
//    bool m_previous_enable_state = false;

    EnabledGate m_enabled_gate;
    TimeEventGate m_time_event_gate;

    Socket<Facet>& m_mode;
    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_duration;
    Socket<Facet>& m_duration_type;
    Socket<Facet>& m_offset;
    Socket<Facet>& m_offset_type;
    Socket<Facet>& m_legato_amount;
    Socket<Facet>& m_sample_and_hold;

};


// ==============================================================================================

template<typename FloatType = float>
struct PulsatorWrapper {
    using Keys = PulsatorNode::PulsatorKeys;

    ParameterHandler parameter_handler;

    Variable<Facet, PulsatorMode> mode{Keys::MODE, parameter_handler, Pulsator::DEFAULT_MODE};
    Sequence<Trigger> trigger{ParameterKeys::TRIGGER, parameter_handler, Voices<Trigger>::empty_like()};
    Sequence<Facet, FloatType> duration{Keys::DURATION, parameter_handler
                                        , Voices<FloatType>::singular(PulsatorParameters::DEFAULT_DURATION)};
    Variable<Facet, DomainType> duration_type{Keys::DURATION_TYPE, parameter_handler
                                              , PulsatorParameters::DEFAULT_DURATION_TYPE};
    Sequence<Facet, FloatType> offset{Keys::OFFSET, parameter_handler
                                      , Voices<FloatType>::singular(PulsatorParameters::DEFAULT_OFFSET)};
    Variable<Facet, DomainType> offset_type{Keys::OFFSET_TYPE, parameter_handler
                                            , PulsatorParameters::DEFAULT_OFFSET_TYPE};
    Sequence<Facet, FloatType> legato_amount{Keys::LEGATO_AMOUNT, parameter_handler
                                             , Voices<FloatType>::singular(PulsatorParameters::DEFAULT_LEGATO_AMOUNT)};
    Sequence<Facet, bool> sample_and_hold{Keys::SAMPLE_AND_HOLD, parameter_handler
                                          , Voices<bool>::singular(PulsatorParameters::DEFAULT_SNH)};
    Sequence<Facet, bool> enabled{ParameterKeys::ENABLED, parameter_handler, Voices<bool>::singular(true)};
    Variable<Facet, std::size_t> num_voices{ParameterKeys::NUM_VOICES, parameter_handler, 0};

    PulsatorNode pulsator_node{Keys::CLASS_NAME
                               , parameter_handler, &mode
                               , &trigger
                               , &duration
                               , &duration_type
                               , &offset
                               , &offset_type
                               , &legato_amount
                               , &sample_and_hold
                               , &enabled
                               , &num_voices};
};


#endif //SERIALISTLOOPER_PULSATOR_H
