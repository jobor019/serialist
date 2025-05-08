#ifndef SERIALIST_RANDOM_NODE_H
#define SERIALIST_RANDOM_NODE_H

#include "core/generative.h"
#include "core/types/trigger.h"
#include "core/types/facet.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "sequence.h"
#include "variable.h"
#include "algo/random/equal_duration_sampling.h"
#include "types/index.h"
#include "types/phase.h"
#include "utility/stateful.h"


namespace serialist {

class RandomHandler {
public:
    using IndexType = Index::IndexType;


    enum class Mode { uniform, weighted, exponential, brownian };


    // Note: only applies when quantization is used
    enum class AvoidRepetitions {
        off            // allow repetitions
        , chordal      // do not allow repetitions within a single Voice / time step (=chord)
        , sequential   // return every element once (over several time steps) before allowing repeats
    };

    static constexpr std::size_t NO_QUANTIZATION = 0;

    static constexpr auto DEFAULT_MODE = Mode::uniform;
    static constexpr auto DEFAULT_REPETITIONS = AvoidRepetitions::off;
    static constexpr std::size_t DEFAULT_CHORD_SIZE = 1;
    static constexpr std::size_t DEFAULT_QUANTIZATION = NO_QUANTIZATION;
    static constexpr auto DEFAULT_BROWNIAN_STEP = 0.05;
    static constexpr auto DEFAULT_EXP_LOWER_BOUND = 0.01;

    static constexpr auto BROWNIAN_START_VALUE = 0.0;
    static constexpr auto EXP_LB_LIMIT = 1e-6;


    explicit RandomHandler(std::optional<int> seed = std::nullopt, double epsilon = EPSILON)
        : m_max(Phase::wrap_point(epsilon)) // note: max is always exclusive
        , m_random(seed)
        , m_exp(DEFAULT_EXP_LOWER_BOUND, m_max, seed) {}


    Voice<double> process(std::size_t chord_size) {
        update_parameter_dependent_state();


        if (*m_mode == Mode::uniform) {
            m_previous_values = uniform(chord_size);
        } else if (*m_mode == Mode::weighted) {
            m_previous_values = weighted(chord_size);
        } else if (*m_mode == Mode::exponential) {
            m_previous_values = exponential(chord_size);
        } else if (*m_mode == Mode::brownian) {
            m_previous_values = brownian(chord_size);
        }

        return m_previous_values;
    }


    void set_mode(Mode mode) { m_mode = mode; }
    void set_repetition_strategy(AvoidRepetitions strategy) { m_repetition_strategy = strategy; }
    void set_max_brownian_step(double step) { m_max_brownian_step = utils::clip(step, 0.0, 1.0); }
    void set_quantization_steps(std::size_t steps) { m_quantization_steps = steps; }


    void set_exp_lower_bound(double lower_bound) {
        auto clipped = utils::clip(lower_bound, EXP_LB_LIMIT, m_max);

        if (utils::equals(m_exp_lower_bound, clipped))
            return;

        m_exp_lower_bound = clipped;
        m_exp.set_lower_bound(m_exp_lower_bound);
    }


    void set_weights(const Vec<double>& weights) {
        m_weights = weights;
        reset_weights();
    }


    void set_seed(uint32_t seed) {
        m_random = Random(seed);
    }

private:
    void update_parameter_dependent_state() {
        // reset_choices on any parameter change is the lazy approach, could be optimized if needed
        if (m_mode.changed() || m_repetition_strategy.changed() || m_quantization_steps.changed()) {
            reset_choices();
        }

        m_mode.clear_flag();
        m_repetition_strategy.clear_flag();
        m_quantization_steps.clear_flag();
    }


    static Voice<double> all_choices(std::size_t num_discrete_steps) {
        return Vec<Index::IndexType>::range(0, static_cast<Index::IndexType>(num_discrete_steps))
                .as_type<double>([num_discrete_steps](const Index::IndexType& i) {
                    return Index::phase_op(i, num_discrete_steps);
                });
    }


    Voice<double> all_weighted_choices() const {
        Vec<std::size_t> valid_indices;

        for (std::size_t i = 0; i < m_weights.size(); ++i) {
            if (!utils::equals(m_weights[i], 0.0)) {
                valid_indices.append(i);
            }
        }

        if (valid_indices.empty()) {
            return {0.0};
        }

        auto num_discrete_steps = m_weights.size();

        return valid_indices.as_type<double>([&num_discrete_steps](const std::size_t& i) {
            return Index::phase_op(static_cast<Index::IndexType>(i), num_discrete_steps);
        });
    }


    void reset_choices() {
        if (use_quantization()) {
            m_current_choices = all_choices(*m_quantization_steps);
        } else {
            m_current_choices.clear();
        }
    }


    void reset_weights() {
        m_current_weights = m_weights;
    }


    void resize_previous_values(std::size_t new_size) {
        if (m_previous_values.empty()) {
            m_previous_values.append(BROWNIAN_START_VALUE);
        }
        m_previous_values.resize_fold(new_size);
    }


    Voice<double> uniform(std::size_t chord_size) {
        // discrete
        if (use_quantization() && *m_quantization_steps > 1) {
            auto r = Voice<double>::allocated(chord_size);
            auto remaining_choices = chord_size;

            if (*m_repetition_strategy == AvoidRepetitions::chordal) {
                reset_choices();
            }

            if (*m_repetition_strategy != AvoidRepetitions::off) {
                auto num_full_cycles = remaining_choices / *m_quantization_steps;
                for (std::size_t i = 0; i < num_full_cycles; ++i) {
                    r.extend(m_random.scrambled(all_choices(*m_quantization_steps)));
                }
                remaining_choices -= num_full_cycles * *m_quantization_steps;
            }

            for (std::size_t i = 0; i < remaining_choices; ++i) {
                r.append(next_choice());
            }
            return r;
        }

        // continuous
        return uniform_continuous_random(chord_size);
    }


    Voice<double> weighted(std::size_t chord_size) {
        if (m_weights.size() <= 1) {
            return {0.0};
        }

        // note: weighted will always be discrete

        auto r = Voice<double>::allocated(chord_size);
        auto remaining_choices = chord_size;

        if (*m_repetition_strategy != AvoidRepetitions::off) {
            auto all_valid_choices = all_weighted_choices();
            assert(!all_valid_choices.empty());

            if (*m_repetition_strategy == AvoidRepetitions::chordal) {
                m_current_choices = all_valid_choices;
            }

            auto num_full_cycles = remaining_choices / all_valid_choices.size();
            for (std::size_t i = 0; i < num_full_cycles; ++i) {
                r.extend(m_random.scrambled(all_valid_choices.cloned()));
            }
            remaining_choices -= num_full_cycles * all_valid_choices.size();
        }

        for (std::size_t i = 0; i < remaining_choices; ++i) {
            r.append(next_weighted_choice());
        }
        return r;
    }


    Voice<double> exponential(std::size_t chord_size) {
        auto uniform_rnd = uniform_continuous_random(chord_size);
        return uniform_rnd.map([this](double u) {
            auto e = m_exp.next(u);
            if (use_quantization()) {
                return Index::quantize(e, *m_quantization_steps);
            }
            return e;
        });
    }


    Voice<double> brownian(std::size_t chord_size) {
        if (m_previous_values.size() != chord_size) {
            resize_previous_values(chord_size);
        }

        if (use_quantization()) {
            auto discrete_step = brownian_discrete_max_steps();

            // note: allow_repetitions only applies in the discrete case
            auto allow_repetitions = *m_repetition_strategy != AvoidRepetitions::off;

            auto r = Voice<double>::allocated(chord_size);
            for (std::size_t i = 0; i < chord_size; ++i) {
                r.append(next_discrete_brownian(m_previous_values[i], discrete_step, allow_repetitions));
            }
            return r;
        }

        auto r = Voice<double>::allocated(chord_size);
        for (std::size_t i = 0; i < chord_size; ++i) {
            r.append(next_continuous_brownian(m_previous_values[i]));
        }
        return r;
    }


    Voice<double> uniform_continuous_random(std::size_t chord_size) {
        auto r = Voice<double>::allocated(chord_size);
        for (std::size_t i = 0; i < chord_size; ++i) {
            r.append(uniform_random());
        }
        return r;
    }


    double uniform_random() {
        return m_random.next() * m_max;
    }


    double next_choice() {
        if (*m_repetition_strategy == AvoidRepetitions::off && use_quantization()) {
            auto q = *m_quantization_steps;
            return Index::phase_op(static_cast<IndexType>(m_random.choice(q)), q);
        }


        if (m_current_choices.empty()) {
            reset_choices();
        }

        auto i = m_random.choice(m_current_choices.size());
        auto choice = *m_current_choices.pop_index(i);
        return choice;
    }


    double next_weighted_choice() {
        assert(m_weights.size() > 1);

        if (*m_repetition_strategy == AvoidRepetitions::off) {
            return index_to_phase(m_random.weighted_choice(m_current_weights), m_current_weights.size());
        }

        if (m_weights.empty()) {
            return 0.0;
        }

        if (utils::equals(m_current_weights.sum(), 0.0)) {
            reset_weights();
        }

        auto i = m_random.weighted_choice(m_current_weights);
        m_current_weights[i] = 0.0;
        return index_to_phase(i, m_weights.size());
    }


    double next_discrete_brownian(double previous_value, IndexType max_steps, bool allow_repetitions) {
        if (*m_quantization_steps <= 1) {
            return 0.0;
        }

        IndexType new_index;

        // note that quantization may have changed since the previous value
        auto previous_index = Index::index_op(previous_value, *m_quantization_steps);

        if (!allow_repetitions) {
            auto n_steps = m_random.choice(static_cast<std::size_t>(max_steps)) + 1;
            auto sign = static_cast<IndexType>(m_random.next() < 0.5 ? -1 : 1);
            new_index = previous_index + static_cast<IndexType>(n_steps) * sign;

        } else {
            auto rand_step = m_random.choice(2 * static_cast<std::size_t>(max_steps) + 1);
            auto num_steps = static_cast<IndexType>(rand_step) - max_steps;
            new_index = previous_index + num_steps;
        }


        auto ub = static_cast<IndexType>(*m_quantization_steps - 1);

        // Handle reflection at boundaries
        if (new_index < 0) {
            new_index = -new_index;
        } else if (new_index >= *m_quantization_steps) {
            new_index = 2 * ub - new_index;
        }

        // value is the same due to reflection
        if (!allow_repetitions && new_index == previous_index) {
            if (new_index == 0) {
                new_index = 1;
            } else if (new_index == ub) {
                new_index = ub - 1;
            } else {
                new_index += static_cast<IndexType>(m_random.next() < 0.5 ? -1 : 1);
            }
        }

        return Index::phase_op(new_index, *m_quantization_steps);
    }


    double next_continuous_brownian(double previous_value) {
        double delta = m_random.next(-m_max_brownian_step, m_max_brownian_step);
        double new_value = previous_value + delta;

        // Handle reflection at boundaries
        if (new_value < 0.0) {
            new_value = -new_value;
        } else if (new_value > m_max) {
            new_value = 2.0 * m_max - new_value;
        }

        return new_value;
    }


    IndexType brownian_discrete_max_steps() const {
        assert(*m_quantization_steps > 0);

        double step_size = m_max / static_cast<double>(*m_quantization_steps);
        if (m_max_brownian_step < step_size) {
            return 1;
        }
        return Index::index_op(m_max_brownian_step, *m_quantization_steps);
    }


    bool use_quantization() const { return *m_quantization_steps > 0; }


    static double index_to_phase(std::size_t index, std::size_t map_size) {
        return Index::phase_op(static_cast<IndexType>(index), map_size);
    }


    double m_max;

    Random m_random;
    EqualDurationSampling m_exp;

    // Parameters
    WithChangeFlag<Mode> m_mode{DEFAULT_MODE};


    WithChangeFlag<AvoidRepetitions> m_repetition_strategy
            {DEFAULT_REPETITIONS}; // does not apply to Mode::brownian or Mode::exponential
    WithChangeFlag<std::size_t> m_quantization_steps{DEFAULT_QUANTIZATION}; // does not apply to Mode::weighted

    double m_max_brownian_step = DEFAULT_BROWNIAN_STEP; // only applies to Mode::brownian
    double m_exp_lower_bound = DEFAULT_EXP_LOWER_BOUND;
    Vec<double> m_weights; // only applies to Mode::weighted


    // State
    Vec<double> m_current_weights; // only used by Mode::weighted
    Vec<double> m_current_choices; // only used by Mode::uniform and Mode::exponential
    Vec<double> m_previous_values;
};


// ==============================================================================================


class RandomNode : public NodeBase<Facet> {
public:
    struct Keys {
        static const inline std::string MODE = "mode";
        static const inline std::string REPETITION_STRATEGY = "repetition_strategy";
        static const inline std::string CHORD_SIZE = "chord_size";
        static const inline std::string QUANTIZATION = "quantization";
        static const inline std::string MAX_BROWNIAN_STEP = "max_brownian_step";
        static const inline std::string EXP_LOWER_BOUND = "exp_lower_bound";
        static const inline std::string WEIGHTS = "weights";

        static const inline std::string CLASS_NAME = "random";
    };


    RandomNode(const std::string& identifier
               , ParameterHandler& parent
               , Node<Trigger>* trigger = nullptr
               , Node<Facet>* mode = nullptr
               , Node<Facet>* repetition_strategy = nullptr
               , Node<Facet>* chord_size = nullptr
               , Node<Facet>* num_quantization_steps = nullptr
               , Node<Facet>* max_brownian_step = nullptr
               , Node<Facet>* exp_lower_bound = nullptr
               , Node<Facet>* weights = nullptr
               , Node<Facet>* enabled = nullptr
               , Node<Facet>* num_voices = nullptr
               , const std::string& class_name = Keys::CLASS_NAME)
        : NodeBase(identifier, parent, enabled, num_voices, class_name)
        , m_trigger(add_socket(param::properties::trigger, trigger))
        , m_mode(add_socket(Keys::MODE, mode))
        , m_repetition_strategy(add_socket(Keys::REPETITION_STRATEGY, repetition_strategy))
        , m_chord_size(add_socket(Keys::CHORD_SIZE, chord_size))
        , m_num_quantization_steps(add_socket(Keys::QUANTIZATION, num_quantization_steps))
        , m_max_brownian_step(add_socket(Keys::MAX_BROWNIAN_STEP, max_brownian_step))
        , m_exp_lower_bound(add_socket(Keys::EXP_LOWER_BOUND, exp_lower_bound))
        , m_weights(add_socket(Keys::WEIGHTS, weights)) {}


    Voices<Facet> process() override {
        if (auto t = pop_time(); !t)
            return m_current_value;

        if (!is_enabled() || !m_trigger.is_connected()) {
            m_current_value = Voices<Facet>::empty_like();
            return m_current_value;
        }

        auto trigger = m_trigger.process();
        if (trigger.is_empty_like())
            return m_current_value;

        auto num_voices = get_voice_count();

        bool resized = num_voices != m_random_handlers.size();
        if (resized) {
            m_random_handlers.resize(num_voices);
        }

        update_parameters(num_voices, resized);

        auto chord_sizes = m_chord_size.process(num_voices).firsts_or(RandomHandler::DEFAULT_CHORD_SIZE);

        trigger.adapted_to(num_voices);

        m_current_value.adapted_to(num_voices);
        for (std::size_t i = 0; i < num_voices; ++i) {
            if (Trigger::contains_pulse_on(trigger[i])) {
                m_current_value[i] = m_random_handlers[i].process(chord_sizes[i]).as_type<Facet>();
            }
        }

        return m_current_value;
    }


    void set_seed(std::size_t seed) {
        for (auto& handler : m_random_handlers) {
            handler.set_seed(seed);
        }
    }

private:
    std::size_t get_voice_count() {
        return voice_count(m_trigger.voice_count()
                           , m_chord_size.voice_count()
                           , m_num_quantization_steps.voice_count());
    }


    void update_parameters(std::size_t num_voices, bool resized) {
        auto mode = m_mode.process().first_or(RandomHandler::DEFAULT_MODE);
        auto reps = m_repetition_strategy.process().first_or(RandomHandler::DEFAULT_REPETITIONS);
        auto steps = m_num_quantization_steps.process(num_voices).firsts_or(RandomHandler::DEFAULT_QUANTIZATION);
        auto brownian_step = m_max_brownian_step.process().first_or(RandomHandler::DEFAULT_BROWNIAN_STEP);
        auto exp_lb = m_exp_lower_bound.process().first_or(RandomHandler::DEFAULT_EXP_LOWER_BOUND);
        auto weights = m_weights.process().firsts_or<double>(0.0);

        m_random_handlers.set(&RandomHandler::set_mode, mode);
        m_random_handlers.set(&RandomHandler::set_repetition_strategy, reps);
        m_random_handlers.set(&RandomHandler::set_quantization_steps, std::move(steps));
        m_random_handlers.set(&RandomHandler::set_max_brownian_step, brownian_step);
        m_random_handlers.set(&RandomHandler::set_exp_lower_bound, exp_lb);
        m_random_handlers.set(&RandomHandler::set_weights, weights);
    }


    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_mode;
    Socket<Facet>& m_repetition_strategy;
    Socket<Facet>& m_chord_size;
    Socket<Facet>& m_num_quantization_steps;
    Socket<Facet>& m_max_brownian_step;
    Socket<Facet>& m_exp_lower_bound;
    Socket<Facet>& m_weights;

    MultiVoiced<RandomHandler, double> m_random_handlers;

    Voices<Facet> m_current_value = Voices<Facet>::empty_like();
};


// ==============================================================================================


template<typename FloatType = double>
struct RandomWrapper {
    using Keys = RandomNode::Keys;

    ParameterHandler ph;

    Sequence<Trigger> trigger{param::properties::trigger, ph, Trigger::pulse_on()};

    Variable<Facet, RandomHandler::Mode> mode{Keys::MODE, ph, RandomHandler::DEFAULT_MODE};
    Variable<Facet, RandomHandler::AvoidRepetitions> repetition_strategy{
        Keys::REPETITION_STRATEGY
        , ph
        , RandomHandler::DEFAULT_REPETITIONS
    };

    Sequence<Facet, std::size_t> chord_size{
        Keys::CHORD_SIZE
        , ph
        , Voices<std::size_t>::singular(RandomHandler::DEFAULT_CHORD_SIZE)
    };

    Sequence<Facet, std::size_t> num_quantization_steps{
        Keys::QUANTIZATION
        , ph
        , Voices<std::size_t>::singular(
            RandomHandler::DEFAULT_QUANTIZATION)
    };
    Variable<Facet, FloatType> max_brownian_step{Keys::MAX_BROWNIAN_STEP, ph, RandomHandler::DEFAULT_BROWNIAN_STEP};

    Variable<Facet, FloatType> exp_lower_bound{Keys::EXP_LOWER_BOUND, ph, RandomHandler::DEFAULT_EXP_LOWER_BOUND};

    Sequence<Facet, FloatType> weights{Keys::WEIGHTS, ph};

    Sequence<Facet, bool> enabled{param::properties::enabled, ph, Voices<bool>::singular(true)};
    Variable<Facet, std::size_t> num_voices{param::properties::num_voices, ph, 0};

    RandomNode random{Keys::CLASS_NAME
                      , ph
                      , &trigger
                      , &mode
                      , &repetition_strategy
                      , &chord_size
                      , &num_quantization_steps
                      , &max_brownian_step
                      , &exp_lower_bound
                      , &weights
                      , &enabled
                      , &num_voices
    };
};
}

#endif //SERIALIST_RANDOM_NODE_H
