
#ifndef SERIALISTLOOPER_PULSATOR_STEREOTYPES_H
#define SERIALISTLOOPER_PULSATOR_STEREOTYPES_H

#include "core/algo/time/pulse.h"
#include "core/algo/time/trigger.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "core/algo/voice/multi_voiced.h"
#include "core/algo/time/jump_gate.h"


class Pulsator : public Flushable<Trigger> {
public:
    virtual void start(double time) = 0;

    virtual Voice<Trigger> stop() = 0;

    virtual bool is_running() const = 0;

    virtual Voice<Trigger> process(double time) = 0;

    virtual Voice<Trigger> handle_time_skip(double new_time) = 0;

};


// ==============================================================================================

template<typename PulsatorType>
class PulsatorBase : public NodeBase<Trigger> {
public:

    PulsatorBase(const std::string& id
                 , ParameterHandler& parent
                 , Node<Facet>* enabled
                 , Node<Facet>* num_voices
                 , const std::string& class_name)
            : NodeBase<Trigger>(id, parent, enabled, num_voices, class_name) {
        static_assert(std::is_base_of_v<Pulsator, PulsatorType>);
    }

    Voices<Trigger> process() override {
        auto t = pop_time();
        if (!t) // process has already been called this cycle
            return m_current_value;

        if (!update_enabled_state()) {
            return m_current_value;
        }

        auto num_voices = get_voice_count();
        Voices<Trigger> output = Voices<Trigger>::zeros(num_voices);

        bool resized = false;
        if (num_voices != m_pulsators.size()) {
            auto initial_size = m_pulsators.size();
            output.merge_uneven(update_size(num_voices), true);
            start(*t, initial_size, num_voices);
            resized = true;
        }

        if (m_jump_gate.poll(*t)) {
            // TODO: This is not a good strategy, shouldn't automatically flush here!!!
            output.merge_uneven(m_pulsators.flush(), true);
            for (auto& pulsator: m_pulsators) {
                pulsator.handle_time_skip(t->get_tick());
            }
        }

        update_parameters(num_voices, resized);

        auto time = t->get_tick();
        for (std::size_t i = 0; i < m_pulsators.size(); ++i) {
            output[i].extend(m_pulsators[i].process(time));
        }

        m_current_value = output;
        return m_current_value;
    }

protected:
    virtual std::size_t get_voice_count() = 0;

    virtual void update_parameters(std::size_t num_voices, bool size_has_changed) = 0;

private:
    /**
     * @return true if enabled, false otherwise
     */
    bool update_enabled_state() {
        if (!is_enabled()) {
            if (m_previous_enable_state) {
                m_current_value = stop();
            } else {
                m_current_value = Voices<Trigger>::empty_like();
            }
            m_previous_enable_state = false;
        }
        m_previous_enable_state = true;

        return m_previous_enable_state;
    }

    Voices<Trigger> update_size(std::size_t num_voices) {
        return m_pulsators.resize(num_voices);
    }

    void start(const TimePoint& t) {
        start(t, 0, m_pulsators.size());
    }

    void start(const TimePoint& t, std::size_t start_index, std::size_t end_index_excl) {
        for (std::size_t i = start_index; i < end_index_excl; ++i) {
            m_pulsators.get_objects()[i].start(t.get_tick());
        }
    }

    Voices<Trigger> stop() {
        auto flushed = Voices<Trigger>::zeros(m_pulsators.size());

        for (std::size_t i = 0; i < m_pulsators.size(); ++i) {
            flushed[i] = m_pulsators.get_objects()[i].stop();
        }

        return flushed;
    }

    MultiVoiced<PulsatorType, Trigger> m_pulsators;

    Voices<Trigger> m_current_value = Voices<Trigger>::empty_like();
    bool m_previous_enable_state = true;

    JumpGate m_jump_gate;
};

#endif //SERIALISTLOOPER_PULSATOR_STEREOTYPES_H
