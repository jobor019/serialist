#ifndef SERIALIST_PULSE_FILTER_H
#define SERIALIST_PULSE_FILTER_H

#include <map>

#include "core/generative.h"
#include "core/types/trigger.h"
#include "core/temporal/pulse.h"
#include "core/types/facet.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "sequence.h"
#include "variable.h"


namespace serialist {


class PulseFilter : public Flushable<Trigger> {
    using PulseState = Held<PulseIdentifier, true>;


    class Strategy {
    public:
        Strategy() = default;
        virtual ~Strategy() = default;
        Strategy(const Strategy&) = delete;
        Strategy& operator=(const Strategy&) = delete;
        Strategy(Strategy&&) noexcept = default;
        Strategy& operator=(Strategy&&) noexcept = default;

        virtual Voice<Trigger> process(Voice<Trigger>&& triggers, PulseState& pulses) = 0;

        virtual Voice<Trigger> activate(PulseState& pulses) { return {}; };
        virtual Voice<Trigger> deactivate(PulseState& pulses) { return {}; };

        static Voice<Trigger> flush(PulseState& pulses) {
            return ids_to_pulse_offs(pulses.flush());
        }

    protected:
        static void register_pulse_ons(const Voice<Trigger>& triggers, PulseState& pulses) {
            for (const auto& trigger : triggers) {
                if (trigger.is_pulse_on()) {
                    register_pulse_on(trigger.get_id(), pulses);
                }
            }
        }


        static Voice<PulseIdentifier> handle_pulse_offs(const Voice<Trigger>& triggers
                                                        , bool release_matched
                                                        , PulseState& pulses) {
            Voice<PulseIdentifier> output;
            for (const auto& trigger : triggers) {
                if (trigger.is_pulse_off()) {
                    if (auto released = handle_pulse_off(trigger.get_id(), release_matched, pulses)) {
                        output.append(*released);
                    }
                }
            }
            return output;
        }


        static void register_pulse_on(std::size_t id, PulseState& pulses) {
            pulses.bind(PulseIdentifier{id});
        }


        static std::optional<PulseIdentifier> handle_pulse_off(std::size_t id, bool release, PulseState& pulses) {
            if (auto p = pulses.find([id](const PulseIdentifier& pulse) { return pulse.id == id; })) {
                if (release) {
                    pulses.release(p->get());
                    return p->get();
                }
                // otherwise: just flag it as triggered without releasing
                p->get().triggered = true;
            }
            return std::nullopt;
        }


        static Voice<Trigger> flush_triggered(PulseState& pulses) {
            return ids_to_pulse_offs(pulses.flush([](const PulseIdentifier& p) { return !p.triggered; }));
        }


        static Voice<Trigger> ids_to_pulse_offs(const Voice<PulseIdentifier>& ids) {
            return ids.as_type<Trigger>([](const PulseIdentifier& p) {
                return Trigger::pulse_off(p.id);
            });
        }

    };

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    /**
     * @brief Pass through both pulse_ons and pulse_offs. Flush any flagged pulse_offs on next pulse_off
     */
    class Open : public Strategy {
    public:
        Voice<Trigger> process(Voice<Trigger>&& triggers, PulseState& pulses) override {
            Voice<Trigger> flushed;

            // We may have lingering pulses from a previous non-immediate closed state
            if (Trigger::contains_pulse_off(triggers)) {
                flushed = flush_triggered(pulses);
            }

            register_pulse_ons(triggers, pulses);

            // we ignore the output here since it's already contained in `triggers`.
            // Using the output from this function only would not work, since it doesn't contain any pulse_ons
            handle_pulse_offs(triggers, true, pulses);

            return merge_without_duplicates(std::move(flushed), std::move(triggers));
        }
    };

    /**
     * @brief Immediately flush all pulse_offs (flagged or not) on activation. Block all pulses.
     */
    class PauseImmediate : public Strategy {
    public:
        Voice<Trigger> process(Voice<Trigger>&& triggers, PulseState& pulses) override {
            assert(pulses.get_held().empty());
            return {};
        }

        Voice<Trigger> activate(PulseState& pulses) override { return flush(pulses); }
    };

    /**
     * @brief Flush all pulse_offs (flagged or not) on next pulse_off. Block all pulses.
     */
    class PauseQuantized : public Strategy {
    public:
        Voice<Trigger> process(Voice<Trigger>&& triggers, PulseState& pulses) override {
            if (Trigger::contains_pulse_off(triggers)) {
                return flush(pulses);
            }

            return {};
        }
    };

    /**
     * @brief Block all pulse_ons. Flag matching pulse_offs but do not output. Flush flagged on deactivation.
     */
    class SustainImmediate : public Strategy {
    public:
        Voice<Trigger> process(Voice<Trigger>&& triggers, PulseState& pulses) override {
            handle_pulse_offs(triggers, false, pulses);
            return {};
        }

        Voice<Trigger> deactivate(PulseState& pulses) override {
            return flush_triggered(pulses);
        }
    };

    /**
     * @brief Block all pulse_ons. Flag matching pulse_offs but do not output.
     */
    class SustainQuantized : public Strategy {
    public:
        Voice<Trigger> process(Voice<Trigger>&& triggers, PulseState& pulses) override {
            handle_pulse_offs(triggers, false, pulses);
            return {};
        }
    };

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    enum class StrategyType {
        open,
        pause_immediate,
        pause_quantized,
        sustain_immediate,
        sustain_quantized,
    };
    
public:
    enum class State { pause, open, sustain };

    static constexpr auto DEFAULT_STATE = State::open;
    static constexpr auto DEFAULT_IMMEDIATE_VALUE = false;


    PulseFilter() = default;
    ~PulseFilter() override = default;

    // Note: Copy constructor and assignment operator are  required since m_strategies isn't copy-constructible.
    // There's however no need to copy any state when copying a PulseFilter - it will be properly initialized with the
    // correct strategy immediately, and we don't want to copy any stored pulses to another voice.
    PulseFilter(const PulseFilter&) {};
    PulseFilter& operator=(const PulseFilter&) {
        return *this;
    };
    PulseFilter(PulseFilter&&) = default;
    PulseFilter& operator=(PulseFilter&&) = default;

    Voice<Trigger> process(Voice<Trigger>&& triggers, State state, bool is_immediate) {
        Voice<Trigger> flushed;

        if (auto new_strategy = parse_strategy_type(state, is_immediate); new_strategy != m_active_strategy) {
            flushed = activate(new_strategy);
        }

        return merge_without_duplicates(std::move(flushed), active_strategy().process(std::move(triggers), m_pulses));
    }


    Voice<Trigger> flush() override {
        return Strategy::flush(m_pulses);
    }


private:

    static StrategyType parse_strategy_type(State state, bool is_immediate) {
        switch (state) {
            case State::open:
                return StrategyType::open;
            case State::pause:
                return is_immediate ? StrategyType::pause_immediate : StrategyType::pause_quantized;
            case State::sustain:
                return is_immediate ? StrategyType::sustain_immediate : StrategyType::sustain_quantized;
            default:
                throw std::invalid_argument("Invalid strategy type");
        }
    }


    Voice<Trigger> activate(StrategyType strategy) {
        Voice<Trigger> output = active_strategy().deactivate(m_pulses);
        m_active_strategy = strategy;
        return merge_without_duplicates(active_strategy().activate(m_pulses), std::move(output));
    }


    static Voice<Trigger> merge_without_duplicates(Voice<Trigger>&& flushed, Voice<Trigger>&& triggers) {
        if (flushed.empty()) return std::move(triggers);
        if (triggers.empty()) return std::move(flushed);

        for (auto flushed_trigger : flushed) {
            if (!triggers.contains(flushed_trigger)) {
                triggers.insert_sorted(flushed_trigger);
            }
        }

        return std::move(triggers);
    }
    

    Strategy& active_strategy() {
        return *m_strategies[m_active_strategy];
    }

    static std::map<StrategyType, std::unique_ptr<Strategy>> make_strategies() {
        std::map<StrategyType, std::unique_ptr<Strategy>> strategies;
        strategies[StrategyType::open] = std::make_unique<Open>();
        strategies[StrategyType::pause_immediate] = std::make_unique<PauseImmediate>();
        strategies[StrategyType::pause_quantized] = std::make_unique<PauseQuantized>();
        strategies[StrategyType::sustain_immediate] = std::make_unique<SustainImmediate>();
        strategies[StrategyType::sustain_quantized] = std::make_unique<SustainQuantized>();
        return strategies;
    }

    std::map<StrategyType, std::unique_ptr<Strategy>> m_strategies = make_strategies();
    StrategyType m_active_strategy = StrategyType::open;
    PulseState m_pulses;
};


// ==============================================================================================

class PulseFilterNode : public NodeBase<Trigger> {
public:
    struct Keys {
        static const inline std::string FILTER_STATE = "filter_state";
        static const inline std::string IMMEDIATE = "immediate";
        static const inline std::string CLASS_NAME = "pulse_filter";
    };


    PulseFilterNode(const std::string& identifier
                    , ParameterHandler& parent
                    , Node<Trigger>* trigger = nullptr
                    , Node<Facet>* filter_state = nullptr
                    , Node<Facet>* immediate = nullptr
                    , Node<Facet>* enabled = nullptr
                    , Node<Facet>* num_voices = nullptr
                    , const std::string& class_name = Keys::CLASS_NAME)
        : NodeBase(identifier, parent, enabled, num_voices, class_name)
        , m_trigger(add_socket(param::properties::trigger, trigger))
        , m_filter_state(add_socket(Keys::FILTER_STATE, filter_state))
        , m_immediate(add_socket(Keys::IMMEDIATE, immediate)) {}


    // TODO: Lots of unnecessary code duplication from MakeNote (and PhasePulsator) => generalize base class

    Voices<Trigger> process() override {
        auto t = pop_time();
        if (!t) {
            return m_current_value;
        }

        bool disabled = is_disabled(*t);
        auto enabled_state = m_enabled_gate.update(!disabled);
        if (auto flushed = handle_enabled_state(enabled_state)) {
            m_current_value = *flushed; // Note: this is empty_like for any disabled time step but the first
            return m_current_value;
        }

        auto trigger = m_trigger.process();
        auto filter_state = m_filter_state.process();
        auto immediate = m_immediate.process().first_or(PulseFilter::DEFAULT_IMMEDIATE_VALUE);

        auto num_voices = voice_count(trigger.size(), filter_state.size());

        auto output = Voices<Trigger>::zeros(num_voices);

        if (num_voices != m_pulse_filters.size()) {
            // Note: after this, output may not have the same size as num_voices, but the size is at least num_voices
            output.merge_uneven(m_pulse_filters.resize(num_voices), true);
        }

        auto triggers = trigger.adapted_to(num_voices);
        auto filter_states = filter_state.adapted_to(num_voices).firsts_or(PulseFilter::DEFAULT_STATE);

        for (std::size_t i = 0; i < num_voices; ++i) {
            output[i].extend(m_pulse_filters[i].process(std::move(triggers[i]), filter_states[i], immediate));
        }

        m_current_value = std::move(output);
        return m_current_value;
    }


    /** (MaxMSP) Extra function for flushing outside the process chain (e.g. when Transport is stopped).
     *           Note that this should never be used in a GenerationGraph, as the objects will be polled at least
     *           once when the transport is stopped.
     *           We need to implement a Socket<Trigger> flush for the GenerationGraph case
     *           (see PhasePulsatorNode for reference)
     *           This is not thread-safe.
     */
    Voices<Trigger> flush() {
        return m_pulse_filters.flush();
    }

private:
    bool is_disabled(const TimePoint& t) {
        return !t.get_transport_running()
               || !is_enabled()
               || !m_trigger.is_connected()
               || !m_filter_state.is_connected();
    }


    // TODO: Unnecessary code duplication from MakeNote (and PhasePulsator) => generalize base class
    /**
     * @return std::nullopt if the Node is enabled, flushed (which may be empty) if the Node is disabled
     *
     */
    std::optional<Voices<Trigger>> handle_enabled_state(EnabledState state) {
        if (state == EnabledState::disabled_this_cycle) {
            return m_pulse_filters.flush();
        } else if (state == EnabledState::disabled_previous_cycle || state == EnabledState::disabled) {
            return Voices<Trigger>::empty_like();
        }
        return std::nullopt;
    }


    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_filter_state;
    Socket<Facet>& m_immediate;

    EnabledGate m_enabled_gate;
    TimeEventGate m_time_event_gate;

    MultiVoiced<PulseFilter, Trigger> m_pulse_filters;

    Voices<Trigger> m_current_value = Voices<Trigger>::empty_like();
};


// ==============================================================================================

template<typename FloatType = double>
struct PulseFilterWrapper {
    using Keys = PulseFilterNode::Keys;

    ParameterHandler ph;
    Sequence<Trigger> trigger{param::properties::trigger, ph, Voices<Trigger>::empty_like()};
    Sequence<Facet, PulseFilter::State> filter_state{Keys::FILTER_STATE
                                            , ph
                                            , Voices<PulseFilter::State>::singular(PulseFilter::DEFAULT_STATE)
    };
    Variable<Facet, bool> immediate{Keys::IMMEDIATE, ph, PulseFilter::DEFAULT_IMMEDIATE_VALUE};

    Sequence<Facet, bool> enabled{param::properties::enabled, ph, Voices<bool>::singular(true)};
    Variable<Facet, std::size_t> num_voices{param::properties::num_voices, ph, 0};
    PulseFilterNode pulse_filter_node{Keys::CLASS_NAME
                                      , ph
                                      , &trigger
                                      , &filter_state
                                      , &immediate
                                      , &enabled
                                      , &num_voices
    };
};
}
#endif //SERIALIST_PULSE_FILTER_H
