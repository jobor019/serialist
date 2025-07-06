#ifndef SERIALIST_PULSE_FILTER_H
#define SERIALIST_PULSE_FILTER_H

#include "core/generative.h"
#include "core/types/trigger.h"
#include "core/temporal/pulse.h"
#include "core/types/facet.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "sequence.h"
#include "variable.h"


namespace serialist {

class PulseFilter : public Flushable<Trigger> {
public:
    enum class Mode { sustain, pause };

    static constexpr auto DEFAULT_MODE = Mode::sustain;
    static constexpr auto DEFAULT_IMMEDIATE_VALUE = true;
    static constexpr auto DEFAULT_CLOSED_STATE = false;
    static constexpr double STATE_OPEN = 1.0;
    static constexpr double STATE_CLOSED = 0.0;


    Voice<Trigger> process(Voice<Trigger>&& triggers, bool is_closed, Mode mode, bool is_immediate) {

        if (mode != m_mode) {
            // For now, we don't need to handle this case gracefully during runtime
            m_mode = mode;
            if (!m_is_first_value) {
                return flush();
            }
        }

        if (is_immediate != m_immediate) {
            // For now, we don't need to handle this case gracefully during runtime
            m_immediate = is_immediate;
            if (!m_is_first_value) {
                return flush();
            }
        }

        // Already open
        if (!m_closed && !is_closed) {
            return process_open(std::move(triggers));
        }

        // Closed in this cycle
        // ReSharper disable once CppDFAConstantConditions
        if (!m_closed && is_closed) {
            m_closed = is_closed;
            return close(std::move(triggers));
        }

        // Opened in this cycle
        // ReSharper disable once CppDFAConstantConditions
        if (m_closed && !is_closed) {
            m_closed = is_closed;
            return open(std::move(triggers));
        }

        // Otherwise: already closed
        return process_closed(std::move(triggers));
    }


    Voice<Trigger> flush() override {
        return ids_to_pulse_offs(m_pulses.flush());
    }

private:
    Voice<Trigger> open(Voice<Trigger>&& triggers) {
        Voice<Trigger> flushed;
        if (m_mode == Mode::sustain && m_immediate) {
            flushed = flush_triggered();
        }

        return merge_without_duplicates(std::move(flushed), process_open(std::move(triggers)));
    }


    Voice<Trigger> close(Voice<Trigger>&& triggers) {
        if (m_mode == Mode::sustain) {
            register_pulse_ons(triggers);

            // Flag any pulse_offs without releasing them
            handle_pulse_offs(triggers, false);

            // Remove all pulse offs from output
            triggers.filter_drain([](const Trigger& t) { return !t.is_pulse_off(); });
            return triggers;
        }

        // mode == pause
        if (m_immediate) {
            return flush();
        } else {
            return process_closed(std::move(triggers));
        }
    }


    Voice<Trigger> process_closed(Voice<Trigger>&& triggers) {
        // Pulse on (all modes): ignore
        //
        // Pulse offs:
        //   sustain (immediate & !immediate): flag matching held pulses as triggered, without releasing them
        //   pause (immediate):                same as above
        //   pause (!immediate):               release any matched held pulses
        bool release_matched = m_mode == Mode::pause && !m_immediate;
        return ids_to_pulse_offs(handle_pulse_offs(triggers, release_matched));
    }


    Voice<Trigger> process_open(Voice<Trigger>&& triggers) {
        // Pulse ons (all modes): process as usual
        //
        // Pulse offs:
        //   sustain (!immediate): we may have lingering pulses from a previous closed state, which should all
        //                         be released on the next pulse_off (independently of matching id)
        //   sustain (immediate):  passthrough
        //   pause (both):         passthrough

        Voice<Trigger> flushed;
        if (m_mode == Mode::sustain && !m_immediate && Trigger::contains_pulse_off(triggers)) {
            flushed = flush_triggered();
        }

        register_pulse_ons(triggers);
        handle_pulse_offs(triggers, true);

        return merge_without_duplicates(std::move(flushed), std::move(triggers));
    }


    void register_pulse_ons(const Voice<Trigger>& triggers) {
        for (const auto& trigger : triggers) {
            if (trigger.is_pulse_on()) {
                register_pulse_on(trigger.get_id());
            }
        }
    }


    Voice<PulseIdentifier> handle_pulse_offs(const Voice<Trigger>& triggers, bool release_matched) {
        Voice<PulseIdentifier> output;
        for (const auto& trigger : triggers) {
            if (trigger.is_pulse_off()) {
                if (auto released = handle_pulse_off(trigger.get_id(), release_matched)) {
                    output.append(*released);
                }
            }
        }
        return output;
    }


    void register_pulse_on(std::size_t id) {
        m_pulses.bind(PulseIdentifier{id});
    }


    std::optional<PulseIdentifier> handle_pulse_off(std::size_t id, bool release) {
        if (auto p = m_pulses.find([id](const PulseIdentifier& pulse) { return pulse.id == id; })) {
            if (release) {
                m_pulses.release(p->get());
                return p->get();
            }
            // otherwise: just flag it as triggered without releasing
            p->get().triggered = true;
        }
        return std::nullopt;
    }


    Voice<Trigger> flush_triggered() {
        return ids_to_pulse_offs(m_pulses.flush([](const PulseIdentifier& p) { return !p.triggered; }));
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


    static Voice<Trigger> ids_to_pulse_offs(const Voice<PulseIdentifier>& ids) {
        return ids.as_type<Trigger>([](const PulseIdentifier& p) {
            return Trigger::pulse_off(p.id);
        });
    }

    bool m_is_first_value = true; // avoid flushing on first value

    Held<PulseIdentifier, true> m_pulses;
    Mode m_mode = DEFAULT_MODE;
    bool m_immediate = DEFAULT_IMMEDIATE_VALUE;

    bool m_closed = DEFAULT_CLOSED_STATE;
};


// ==============================================================================================

class PulseFilterNode : public NodeBase<Trigger> {
public:
    struct Keys {
        static const inline std::string FILTER_STATE = "filter_state";
        static const inline std::string MODE = "mode";
        static const inline std::string IMMEDIATE = "immediate";
        static const inline std::string CLASS_NAME = "pulse_filter";
    };


    PulseFilterNode(const std::string& identifier
                    , ParameterHandler& parent
                    , Node<Trigger>* trigger = nullptr
                    , Node<Facet>* filter_state = nullptr
                    , Node<Facet>* mode = nullptr
                    , Node<Facet>* immediate = nullptr
                    , Node<Facet>* enabled = nullptr
                    , Node<Facet>* num_voices = nullptr
                    , const std::string& class_name = Keys::CLASS_NAME)
        : NodeBase(identifier, parent, enabled, num_voices, class_name)
        , m_trigger(add_socket(param::properties::trigger, trigger))
        , m_filter_state(add_socket(Keys::FILTER_STATE, filter_state))
        , m_mode(add_socket(Keys::MODE, mode))
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

        auto mode = m_mode.process().first_or(PulseFilter::DEFAULT_MODE);
        auto immediate = m_immediate.process().first_or(PulseFilter::DEFAULT_IMMEDIATE_VALUE);

        auto num_voices = voice_count(trigger.size(), filter_state.size());

        auto output = Voices<Trigger>::zeros(num_voices);

        if (num_voices != m_pulse_filters.size()) {
            // Note: after this, output may not have the same size as num_voices, but the size is at least num_voices
            output.merge_uneven(m_pulse_filters.resize(num_voices), true);
        }

        trigger.adapted_to(num_voices);
        auto is_closed = filter_state
                .adapted_to(num_voices)
                .as_type<bool>([](const Facet& f) {
                    return utils::equals(static_cast<double>(f), PulseFilter::STATE_CLOSED);
                })
                .firsts_or(false);

        for (std::size_t i = 0; i < num_voices; ++i) {
            output[i].extend(m_pulse_filters[i].process(std::move(trigger[i]), is_closed[i], mode, immediate));
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
    Socket<Facet>& m_mode;
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
    Sequence<Facet, FloatType> filter_state{Keys::FILTER_STATE
                                            , ph
                                            , Voices<FloatType>::singular(PulseFilter::STATE_OPEN)
    };
    Variable<Facet, PulseFilter::Mode> mode{Keys::MODE, ph, PulseFilter::DEFAULT_MODE};
    Variable<Facet, bool> immediate{Keys::IMMEDIATE, ph, PulseFilter::DEFAULT_IMMEDIATE_VALUE};

    Sequence<Facet, bool> enabled{param::properties::enabled, ph, Voices<bool>::singular(true)};
    Variable<Facet, std::size_t> num_voices{param::properties::num_voices, ph, 0};
    PulseFilterNode pulse_filter_node{Keys::CLASS_NAME
                                      , ph
                                      , &trigger
                                      , &filter_state
                                      , &mode
                                      , &immediate
                                      , &enabled
                                      , &num_voices
    };
};
}
#endif //SERIALIST_PULSE_FILTER_H
