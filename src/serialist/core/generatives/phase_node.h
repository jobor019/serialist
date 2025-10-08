
#ifndef SERIALIST_PHASE_NODE_H
#define SERIALIST_PHASE_NODE_H

#include "core/algo/random/random.h"
#include "core/temporal/filters.h"
#include "core/temporal/phase_accumulator.h"
#include "core/types/facet.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "core/types/trigger.h"
#include "core/collections/multi_voiced.h"
#include "sequence.h"
#include "variable.h"

namespace serialist {


class PhaseNode : public NodeBase<Facet> {
public:
    struct Keys {
        static const inline std::string TRIGGER = "trigger";
        static const inline std::string MODE = "mode";
        static const inline std::string PERIOD = "period";
        static const inline std::string PERIOD_TYPE = "period_type";
        static const inline std::string OFFSET = "offset";
        static const inline std::string OFFSET_TYPE = "offset_type";
        static const inline std::string STEP_SIZE = "step_size";
        static const inline std::string RESET = "reset";

        static const inline std::string CLASS_NAME = "phase";
    };

    PhaseNode(const std::string& id
                   , ParameterHandler& parent
                   , Node<Trigger>* trigger = nullptr
                   , Node<Facet>* mode = nullptr
                   , Node<Facet>* period = nullptr
                   , Node<Facet>* period_type = nullptr
                   , Node<Facet>* offset = nullptr
                   , Node<Facet>* offset_type = nullptr
                   , Node<Facet>* step_size = nullptr
                   , Node<Trigger>* reset = nullptr
                   , Node<Facet>* enabled = nullptr
                   , Node<Facet>* num_voices = nullptr)
            : NodeBase<Facet>(id, parent, enabled, num_voices, Keys::CLASS_NAME)
              , m_trigger(add_socket(Keys::TRIGGER, trigger))
              , m_mode(add_socket(Keys::MODE, mode))
              , m_period(add_socket(Keys::PERIOD, period))
              , m_period_type(add_socket(Keys::PERIOD_TYPE, period_type))
              , m_offset(add_socket(Keys::OFFSET, offset))
              , m_offset_type(add_socket(Keys::OFFSET_TYPE, offset_type))
              , m_step_size(add_socket(Keys::STEP_SIZE, step_size))
              , m_reset(add_socket(Keys::RESET, reset)) {}

    Voices<Facet> process() override {
        auto t = pop_time();
        if (!t)
            return m_current_value;

        if (!is_enabled() || !m_trigger.is_connected()) {
            m_current_value = Voices<Facet>::empty_like();
            return m_current_value;
        }

        auto reset_triggers = m_reset.process();
        if (Trigger::contains_pulse_on(reset_triggers)) {
            reset();
        }

        auto trigger = m_trigger.process();
        if (trigger.is_empty_like())
            return m_current_value;

        auto num_voices = get_voice_count();

        bool resized = num_voices != m_phases.size();
        if (resized) {
            m_phases.resize(num_voices);
        }

        update_parameters(num_voices, resized);

        trigger.adapted_to(num_voices);

        auto output = Voices<Facet>::zeros(num_voices);
        for (std::size_t i = 0; i < num_voices; ++i) {
            bool has_trigger = Trigger::contains_pulse_on(trigger[i]);
            output[i].append(static_cast<Facet>(m_phases[i].process(*t, has_trigger)));
        }

        m_current_value = std::move(output);
        return m_current_value;
    }

private:
    std::size_t get_voice_count() {
        return voice_count(m_trigger.voice_count()
                           , m_period.voice_count()
                           , m_offset.voice_count()
                           , m_step_size.voice_count());
    }

    void update_parameters(std::size_t num_voices, bool resized) {
        m_phases.set(&PhaseAccumulator::set_mode
                          , adapted(m_mode.process(), num_voices, PhaseAccumulator::DEFAULT_MODE));

        // TODO: This has_changed is completely redundant: we've already called process() in get_voice_count,
        //       so has_changed is always false here
        if (resized || m_period.has_changed() || m_period_type.has_changed()) {
            auto period = m_period.process().adapted_to(num_voices).firsts_or(PaParameters::DEFAULT_PERIOD);
            auto period_type = m_period_type.process().first_or(PaParameters::DEFAULT_PERIOD_TYPE);

            for (std::size_t i = 0; i < num_voices; ++i) {
                m_phases[i].set_period(DomainDuration{period[i], period_type});
            }
        }

        if (resized || m_offset.has_changed() || m_offset_type.has_changed()) {
            auto offset = m_offset.process().adapted_to(num_voices).firsts_or(PaParameters::DEFAULT_OFFSET);
            auto offset_type = m_offset_type.process().first_or(PaParameters::DEFAULT_OFFSET_TYPE);

            for (std::size_t i = 0; i < num_voices; ++i) {
                m_phases[i].set_offset(DomainDuration{offset[i], offset_type});
            }
        }

        m_phases.set(&PhaseAccumulator::set_step_size
                     , adapted(m_step_size.process(), num_voices, PaParameters::DEFAULT_STEP_SIZE));
    }

    void reset() {
        for (auto& oscillator : m_phases) {
            oscillator.reset();
        }
    }


    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_mode;
    Socket<Facet>& m_period;
    Socket<Facet>& m_period_type;
    Socket<Facet>& m_offset;
    Socket<Facet>& m_offset_type;
    Socket<Facet>& m_step_size;
    Socket<Trigger>& m_reset;

    MultiVoiced<PhaseAccumulator, double> m_phases;

    Voices<Facet> m_current_value = Voices<Facet>::empty_like();
};


// ==============================================================================================

template<typename FloatType = double>
struct PhaseWrapper {
    using Keys = PhaseNode::Keys;

    ParameterHandler ph;

    Sequence<Trigger> trigger{Keys::TRIGGER, ph, Trigger::pulse_on()};

    Variable<Facet, PaMode> mode{Keys::MODE, ph, PhaseAccumulator::DEFAULT_MODE};

    Sequence<Facet, FloatType> period{Keys::PERIOD, ph, Voices<FloatType>::singular(PaParameters::DEFAULT_PERIOD)};
    Variable<Facet, DomainType> period_type{Keys::PERIOD_TYPE, ph, PaParameters::DEFAULT_PERIOD_TYPE};
    Sequence<Facet, FloatType> offset{Keys::OFFSET, ph, Voices<FloatType>::singular(PaParameters::DEFAULT_OFFSET)};
    Variable<Facet, DomainType> offset_type{Keys::OFFSET_TYPE, ph, PaParameters::DEFAULT_OFFSET_TYPE};
    Sequence<Facet, FloatType> step_size{Keys::STEP_SIZE, ph
            , Voices<FloatType>::singular(static_cast<FloatType>(PaParameters::DEFAULT_STEP_SIZE))
    };

    Sequence<Trigger> reset_trigger{Keys::RESET, ph};

    Sequence<Facet, bool> enabled{param::properties::enabled, ph, Voices<bool>::singular(true)};
    Variable<Facet, std::size_t> num_voices{param::properties::num_voices, ph, 0};

    PhaseNode phase_node{Keys::CLASS_NAME
                         , ph
                         , &trigger
                         , &mode
                         , &period
                         , &period_type
                         , &offset
                         , &offset_type
                         , &step_size
                         , &reset_trigger
                         , &enabled
                         , &num_voices
    };
};

} // namespace serialist

#endif //SERIALIST_PHASE_NODE_H
