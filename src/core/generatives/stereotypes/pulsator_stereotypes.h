
#ifndef SERIALISTLOOPER_PULSATOR_STEREOTYPES_H
#define SERIALISTLOOPER_PULSATOR_STEREOTYPES_H

#include "core/algo/time/pulse.h"
#include "core/algo/time/trigger.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "core/algo/voice/multi_voiced.h"
#include "core/algo/time/jump_gate.h"
#include "core/algo/time/time_point.h"


class Pulsator : public Flushable<Trigger> {
public:
    virtual void start(const TimePoint& time, const std::optional<DomainTimePoint>& first_pulse_time) = 0;

    virtual Voice<Trigger> stop() = 0;

    virtual bool is_running() const = 0;

    virtual Voice<Trigger> handle_external_triggers(const TimePoint& time, const Voice<Trigger>& triggers) = 0;

    virtual Voice<Trigger> poll(const TimePoint& time) = 0;

    virtual Voice<Trigger> handle_time_skip(const TimePoint& new_time) = 0;

    virtual Vec<Pulse> export_pulses() = 0;

    virtual void import_pulses(const Vec<Pulse>& pulses) = 0;

    virtual std::optional<DomainTimePoint> next_scheduled_pulse_on() = 0;

};



// ==============================================================================================

template<typename PulsatorType>
class PulsatorNodeBase : public NodeBase<Trigger> {
public:

    PulsatorNodeBase(const std::string& id
                     , ParameterHandler& parent
                     , Node<Facet>* enabled
                     , Node<Facet>* num_voices
                     , const std::string& class_name)
            : NodeBase<Trigger>(id, parent, enabled, num_voices, class_name) {
        static_assert(std::is_base_of_v<Pulsator, PulsatorType>);
        static_assert(std::is_copy_constructible_v<PulsatorType>); // required due to voice resizing
    }

    /**
     * Note: can be overridden if a less standardized procedure is required
     */
    Voices<Trigger> process() override {
        auto t = pop_time();
        if (!t) // process has already been called this cycle
            return m_current_value;

        if (!update_enabled_state(*t)) {
            return m_current_value;
        }

        auto num_voices = get_voice_count();
        Voices<Trigger> output = Voices<Trigger>::zeros(num_voices);

        bool resized = false;
        if (auto flushed = update_size(num_voices, *t)) {
            if (!flushed->is_empty_like()) {
                // from this point on, size of output may be different from num_voices,
                //   but this is the only point where resizing should be allowed
                output.merge_uneven(*flushed, true);
            }
            resized = true;
        }

        if (auto flushed = handle_jump_gate(*t)) {
            output.merge_uneven(*flushed, false);
        }

        update_parameters(num_voices, resized);

        auto incoming_triggers = get_incoming_triggers(*t, num_voices);

        for (std::size_t i = 0; i < m_pulsators.size(); ++i) {
            output[i].extend(m_pulsators[i].handle_external_triggers(*t, incoming_triggers[i]));
            output[i].extend(m_pulsators[i].poll(*t));
        }

        m_current_value = output;
        return m_current_value;
    }

protected:
    virtual std::size_t get_voice_count() = 0;

    virtual void update_parameters(std::size_t num_voices, bool size_has_changed) = 0;

    virtual Voices<Trigger> get_incoming_triggers(const TimePoint& t, std::size_t num_voices) = 0;

    std::optional<Voices<Trigger>> handle_jump_gate(const TimePoint& t) {
        if (m_jump_gate.poll(t)) {
            // TODO: This is not a good strategy, shouldn't automatically flush all here!!!
            auto flushed = m_pulsators.flush();
            for (auto& pulsator: m_pulsators) {
                pulsator.handle_time_skip(t);
            }
            return flushed;
        }
        return std::nullopt;
    }

    /**
    * @return returns current enable state
    */
    bool update_enabled_state(const TimePoint& t) {
        bool enabled = is_enabled();
        if (!enabled) {
            if (m_previous_enable_state) {
                m_current_value = stop();
            } else {
                m_current_value = Voices<Trigger>::empty_like();
            }
            m_previous_enable_state = false;

        } else if (!m_previous_enable_state) {
            // enabled now but wasn't before => need to start all pulsators
            start(t);
        }

        m_previous_enable_state = enabled;

        return enabled;
    }

    /**
     * @return flushed triggers (Voices<Trigger>) if num_voices has changed, std::nullopt otherwise
     *         note that the flushed triggers will have the same size as the previous num_voices, hence
     *         merge_uneven(.., true) is required
     */
    std::optional<Voices<Trigger>> update_size(std::size_t num_voices, const TimePoint& t) {
        if (num_voices != m_pulsators.size()) {
            auto initial_size = m_pulsators.size();
            auto flushed = m_pulsators.resize(num_voices);
            start(t, initial_size, num_voices);

            return flushed;
        }

        return std::nullopt;
    }

    void start(const TimePoint& t) {
        start(t, 0, m_pulsators.size());
    }

    void start(const TimePoint& t, std::size_t start_index, std::size_t end_index_excl) {
        for (std::size_t i = start_index; i < end_index_excl; ++i) {
            m_pulsators.get_objects()[i].start(t, std::nullopt);
        }
    }

    Voices<Trigger> stop() {
        auto flushed = Voices<Trigger>::zeros(m_pulsators.size());

        for (std::size_t i = 0; i < m_pulsators.size(); ++i) {
            flushed[i] = m_pulsators.get_objects()[i].stop();
        }

        return flushed;
    }

    const MultiVoiced<PulsatorType, Trigger>& pulsators() const { return m_pulsators; }

    MultiVoiced<PulsatorType, Trigger>& pulsators() { return m_pulsators; }

    const Voices<Trigger>& current_value() const {
        return m_current_value;
    }

    Voices<Trigger>& current_value() {
        return m_current_value;
    }

private:
    MultiVoiced<PulsatorType, Trigger> m_pulsators;

    Voices<Trigger> m_current_value = Voices<Trigger>::empty_like();
    bool m_previous_enable_state = false;

    JumpGate m_jump_gate;
};

#endif //SERIALISTLOOPER_PULSATOR_STEREOTYPES_H
