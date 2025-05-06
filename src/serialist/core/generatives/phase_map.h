#ifndef SERIALIST_PHASE_MAP_H
#define SERIALIST_PHASE_MAP_H
#include "sequence.h"
#include "variable.h"
#include "policies/parameter_policy.h"
#include "policies/socket_policy.h"
#include "stereotypes/base_stereotypes.h"
#include "core/types/phase.h"


namespace serialist {
class PhaseMap {
public:
    static constexpr double DEFAULT_DURATION = 1.0;
    static const inline Voice<double> DEFAULT_DURATIONS{DEFAULT_DURATION};

    PhaseMap() {
        reset_durations();
    }


    std::optional<Phase> process(const Phase& cursor) {
        auto it = std::lower_bound(m_cumsum.begin(), m_cumsum.end(), cursor.get());
        auto index = std::distance(m_cumsum.begin(), it);

        if (m_is_pause[index]) {
            return std::nullopt;
        }

        // Compute relative position within the segment
        double prev_sum = index == 0 ? 0.0 : m_cumsum[index - 1];
        double position_in_segment = (cursor.get() - prev_sum) / m_durations[index];

        return Phase(position_in_segment);
    }


    void set_durations(Voice<double>&& durations) {
        // Shorthand for tuplets, e.g. 3 = [1,1,1], 5 = [1,1,1,1,1], etc.
        if (durations.size() == 1) {
            // Having N pauses wouldn't be meaningful here, hence std::abs
            auto n = std::round(std::abs(durations[0]));

            durations = Vec<double>::repeated(static_cast<std::size_t>(n), 1.0/n);

        } else {
            durations.filter([](double d) { return !utils::equals(d, 0.0); })
                     .normalize_l1();
        }

        // Handles all the cases where durations is empty: initial input was empty, n was 0 or all durations were 0
        if (durations.empty()) {
            reset_durations();
            return;
        }

        auto size = durations.size();
        auto sum = 0.0;
        auto cumsum = Voice<double>::allocated(durations.size());
        auto is_pause = Voice<bool>::allocated(durations.size());

        for (std::size_t i = 0; i < size; ++i) {
            sum += std::abs(durations[i]);
            cumsum.append(sum);
            is_pause.append(durations[i] <= 0.0);
        }

        m_durations = std::move(durations);
        m_cumsum = std::move(cumsum);
        m_is_pause = std::move(is_pause);
    }

private:
    void reset_durations() {
        m_durations = DEFAULT_DURATIONS;
        m_cumsum = m_durations;
        m_is_pause = Voice<bool>::zeros(m_durations.size());
    }


    Voice<double> m_durations;

    Voice<double> m_cumsum;
    Voice<bool> m_is_pause;
};


class PhaseMapNode : public NodeBase<Facet> {
public:
    struct Keys {
        static const inline std::string CURSOR = "cursor";
        static const inline std::string DURATIONS = "durations";
        static const inline std::string CLASS_NAME = "phase_map";
    };


    PhaseMapNode(const std::string& id
                 , ParameterHandler& parent
                 , Node<Trigger>* trigger = nullptr
                 , Node* cursor = nullptr
                 , Node* durations = nullptr
                 , Node* enabled = nullptr
                 , Node* num_voices = nullptr)
        : NodeBase(id, parent, enabled, num_voices, Keys::CLASS_NAME)
        , m_trigger(add_socket(param::properties::trigger, trigger))
        , m_cursor(add_socket(Keys::CURSOR, cursor))
        , m_durations(add_socket(Keys::DURATIONS, durations)) {}


    Voices<Facet> process() override {
        if (!pop_time())
            return m_current_value;


        if (!is_enabled() || !m_trigger.is_connected() || !m_cursor.is_connected()) {
            m_current_value = Voices<Facet>::empty_like();
            return m_current_value;
        }

        if (m_trigger.process().is_empty_like())
            return m_current_value;

        auto num_voices = get_voice_count();

        auto resized = num_voices != m_phase_maps.size();
        if (resized) {
            m_phase_maps.resize(num_voices);
        }

        update_parameters(num_voices, resized);

        auto cursor_input = m_cursor.process();
        if (cursor_input.is_empty_like()) {
            m_current_value = Voices<Facet>::empty_like();
            return m_current_value;
        }


        auto cursors = cursor_input.adapted_to(num_voices)
                .firsts_or(0.0)
                .as_type<Phase>([](auto phase) { return Phase(phase); });

        auto output = Voices<Facet>::zeros(num_voices);
        for (std::size_t i = 0; i < m_phase_maps.size(); ++i) {
            if (auto phase = m_phase_maps[i].process(Phase(cursors[i]))) {
                output[i].append(static_cast<Facet>(phase->get()));
            } // else: pause, output[i] will be empty_like
        }

        m_current_value = std::move(output);
        return m_current_value;
    }

private:
    std::size_t get_voice_count() {
        return voice_count(m_cursor.voice_count(), m_durations.voice_count());
    }


    void update_parameters(std::size_t num_voices, bool size_has_changed) {
        if (size_has_changed || m_durations.has_changed()) {
            auto durations = m_durations.process().adapted_to(num_voices).as_type<double>();
            m_phase_maps.set(&PhaseMap::set_durations, std::move(durations));
        }
    }


    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_cursor;
    Socket<Facet>& m_durations;

    MultiVoiced<PhaseMap, double> m_phase_maps;

    Voices<Facet> m_current_value = Voices<Facet>::empty_like();
};


template<typename FloatType = double>
struct PhaseMapWrapper {
    using Keys = PhaseMapNode::Keys;

    ParameterHandler parameter_handler;
    Variable<Trigger> trigger{param::properties::trigger, parameter_handler, Trigger::pulse_on()};
    Sequence<Facet, FloatType> cursor{Keys::CURSOR, parameter_handler, Voices<FloatType>::singular(0.0)};
    Sequence<Facet, FloatType> durations{Keys::DURATIONS
                                         , parameter_handler
                                         , Voices<FloatType>::transposed(PhaseMap::DEFAULT_DURATIONS)
    };

    Sequence<Facet, bool> enabled{param::properties::enabled, parameter_handler, Voices<bool>::singular(true)};
    Variable<Facet, std::size_t> num_voices{param::properties::num_voices, parameter_handler, 0};

    PhaseMapNode phase_map_node{Keys::CLASS_NAME
                                , parameter_handler
                                , &trigger
                                , &cursor
                                , &durations
                                , &enabled
                                , &num_voices
    };
};
}

#endif //SERIALIST_PHASE_MAP_H
