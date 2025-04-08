#ifndef SERIALIST_RANDOM_NODE_H
#define SERIALIST_RANDOM_NODE_H

#include "core/types/event.h"
#include "core/generative.h"
#include "core/types/trigger.h"
#include "core/types/facet.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "sequence.h"
#include "variable.h"
#include "algo/random/equal_duration_sampling.h"
#include "types/phase.h"
#include "utility/stateful.h"


namespace serialist {
// ==============================================================================================

class RandomHandler {
public:
    enum class Mode { uniform, weighted, exponential, brownian };


    // Note: only applies when quantization is used
    enum class AvoidRepetitions {
        off            // allow repetitions
        , chordal      // do not allow repetitions within a single Voice / time step (=chord)
        , sequential   // return every element once (over several time steps) before allowing repeats
    };

    static constexpr auto DEFAULT_MODE = Mode::uniform;
    static constexpr auto DEFAULT_REPETITIONS = AvoidRepetitions::off;
    static constexpr std::size_t DEFAULT_CHORD_SIZE = 1;
    static constexpr auto DEFAULT_USE_QUANTIZATION = false;
    static constexpr std::size_t DEFAULT_NUM_QUANTIZATION_STEPS = 4;
    static constexpr auto DEFAULT_MAX_BROWNIAN_STEP = 0.25;


    explicit RandomHandler(std::optional<int> seed = std::nullopt, double epsilon = Phase::EPSILON)
        : m_max(Phase::wrap_point(epsilon))
        , m_random(seed)
        , m_exp(0, m_max, seed) {}


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
    void set_max_brownian_step(double step) { m_max_brownian_step = std::max(0.0, step); }


    void set_quantization_steps(std::optional<std::size_t> steps) {
        assert(!steps || *steps > 0);
        m_quantization_steps = steps;
    }


    void set_weights(const Vec<double>& weights) {
        m_weights = weights;
        reset_weights();
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


    Voice<double> all_choices(std::size_t num_discrete_steps) const {
        return Vec<double>::linspace(0, m_max, std::max(static_cast<std::size_t>(1), num_discrete_steps));
    }


    double scale_to_max(double x) const { return x * m_max; }


    double scale_to_max(std::size_t index, std::size_t num_steps) const {
        return static_cast<double>(index) / static_cast<double>(num_steps) * m_max;
    }


    void reset_choices() {
        if (*m_quantization_steps) {
            m_current_choices = all_choices(m_quantization_steps->value());
        } else {
            m_current_choices.clear();
        }
    }


    void reset_weights() {
        m_current_weights = m_weights;
    }


    void resize_previous_values(std::size_t new_size) {
        m_previous_values.resize_fold(new_size);
    }


    Voice<double> uniform(std::size_t chord_size) {
        if (*m_quantization_steps && *m_quantization_steps > 1) {
            if (*m_repetition_strategy != AvoidRepetitions::off && chord_size >= *m_quantization_steps) {
                return m_random.scrambled(all_choices(m_quantization_steps->value()));
            }

            if (*m_repetition_strategy == AvoidRepetitions::chordal) {
                reset_choices();
            }

            auto r = Voice<double>::allocated(chord_size);
            for (std::size_t i = 0; i < chord_size; ++i) {
                r.append(next_choice());
            }
            return r;
        }

        // Mode::continuous
        auto r = Voice<double>::allocated(chord_size);
        for (std::size_t i = 0; i < chord_size; ++i) {
            r.append(uniform_random());
        }
        return r;
    }


    Voice<double> weighted(std::size_t chord_size) {
        if (m_weights.size() <= 1) {
            return uniform(chord_size);
        }

        // note: weighted will always be discrete

        if (*m_repetition_strategy != AvoidRepetitions::off && chord_size >= m_weights.size()) {
            return m_random.scrambled(all_choices(m_weights.size()));
        }

        if (*m_repetition_strategy == AvoidRepetitions::chordal) {
            reset_choices();
        }

        auto r = Voice<double>::allocated(chord_size);
        for (std::size_t i = 0; i < chord_size; ++i) {
            r.append(next_choice());
        }
        return r;
    }


    Voice<double> exponential(std::size_t chord_size) {
        auto uniform_rnd = uniform(chord_size);
        return uniform_rnd.map([this](double u) {
            return m_exp.next(u);
        });
    }


    Voice<double> brownian(std::size_t chord_size) {
        if (m_previous_values.size() != chord_size) {
            resize_previous_values(chord_size);
        }

        std::optional<std::pair<std::size_t, double>> discrete_step;
        if (*m_quantization_steps) {
            discrete_step = brownian_discrete_step();
        } else {
            discrete_step = std::nullopt;
        }

        auto allow_repetitions = *m_repetition_strategy != AvoidRepetitions::off;

        auto r = Voice<double>::allocated(chord_size);
        for (std::size_t i = 0; i < chord_size; ++i) {
            r.append(next_brownian(m_previous_values[i], discrete_step, allow_repetitions));
        }
        return r;
    }


    double uniform_random() {
        return m_random.next() * m_max;
    }


    double next_choice() {
        if (*m_repetition_strategy == AvoidRepetitions::off && *m_quantization_steps) {
            auto q = m_quantization_steps->value();
            return scale_to_max(m_random.choice(q), q);
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
            return scale_to_max(m_random.weighted_choice(m_current_weights), m_current_weights.size());
        }

        if (m_weights.empty()) {
            return next_choice();
        }

        if (m_current_weights.empty()) {
            reset_weights();
        }

        auto i = m_random.weighted_choice(m_current_weights);
        m_current_weights.pop_index(i);
        return scale_to_max(i, m_current_weights.size());
    }


    double next_brownian(double previous_value
                         , const std::optional<std::pair<std::size_t, double>>& discrete_step
                         , bool allow_repetitions) {
        double new_value;

        if (discrete_step) {
            auto [max_num_steps, step_size] = *discrete_step;
            // quantization may have changed since the previous value
            previous_value = quantize(previous_value, step_size, true);

            if (allow_repetitions) {
                auto num_steps = static_cast<double>(m_random.choice(max_num_steps) + 1);
                auto is_negative = m_random.next() < 0.5;
                auto delta = num_steps * step_size * (is_negative ? -1.0 : 1.0);

                new_value = previous_value + delta;
            } else {
                auto num_steps = m_random.choice(2 * max_num_steps + 1);
                auto delta = (static_cast<double>(num_steps) - static_cast<double>(max_num_steps)) * step_size;
                new_value = previous_value + delta;
            }
        } else {
            // continuous: note that we ignore allow_repetitions here
            double delta = m_random.next(-m_max_brownian_step, m_max_brownian_step);
            new_value = previous_value + delta;
        }

        // Handle reflection at boundaries
        if (new_value < 0.0) {
            new_value = -new_value;
        } else if (new_value > m_max) {
            new_value = 2.0 * m_max - new_value;
        }

        return new_value;
    }


    std::pair<std::size_t, double> brownian_discrete_step() const {
        assert(*m_quantization_steps);

        double step_size = m_max / static_cast<double>(m_quantization_steps->value());
        if (m_max_brownian_step < step_size) {
            return {1u, step_size};
        }

        auto max_step = quantize(m_max_brownian_step, step_size, false);
        auto num_steps = static_cast<std::size_t>(std::round(max_step / step_size));
        return {num_steps, max_step};
    }


    double quantize(double v, double step_size, bool round = false) const {
        if (round)
            return std::round(v / step_size) * step_size * m_max;

        return std::floor(v / step_size) * step_size;
    }


    double m_max;

    Random m_random;
    EqualDurationSampling m_exp;

    // Parameters
    WithChangeFlag<Mode> m_mode{DEFAULT_MODE};


    WithChangeFlag<AvoidRepetitions> m_repetition_strategy{DEFAULT_REPETITIONS}; // does not apply to Mode::brownian
    WithChangeFlag<std::optional<std::size_t>> m_quantization_steps{std::nullopt}; // does not apply to Mode::weighted

    double m_max_brownian_step = DEFAULT_MAX_BROWNIAN_STEP; // only applies to Mode::brownian
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
        static const inline std::string QUANTIZE = "quantize";
        static const inline std::string NUM_QUANTIZATION_STEPS = "num_quantization_steps";
        static const inline std::string MAX_BROWNIAN_STEP = "max_brownian_step";
        static const inline std::string WEIGHTS = "weights";

        static const inline std::string CLASS_NAME = "random";
    };


    RandomNode(const std::string& identifier
               , ParameterHandler& parent
               , Node<Trigger>* trigger = nullptr
               , Node<Facet>* mode = nullptr
               , Node<Facet>* repetition_strategy = nullptr
               , Node<Facet>* chord_size = nullptr
               , Node<Facet>* quantize = nullptr
               , Node<Facet>* num_quantization_steps = nullptr
               , Node<Facet>* max_brownian_step = nullptr
               , Node<Facet>* weights = nullptr
               , Node<Facet>* enabled = nullptr
               , Node<Facet>* num_voices = nullptr
               , const std::string& class_name = Keys::CLASS_NAME)
        : NodeBase(identifier, parent, enabled, num_voices, class_name)
        , m_trigger(add_socket(param::properties::trigger, trigger))
        , m_mode(add_socket(Keys::MODE, mode))
        , m_repetition_strategy(add_socket(Keys::REPETITION_STRATEGY, repetition_strategy))
        , m_chord_size(add_socket(Keys::CHORD_SIZE, chord_size))
        , m_quantize(add_socket(Keys::QUANTIZE, quantize))
        , m_num_quantization_steps(add_socket(Keys::NUM_QUANTIZATION_STEPS, num_quantization_steps))
        , m_max_brownian_step(add_socket(Keys::MAX_BROWNIAN_STEP, max_brownian_step))
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

        auto output = Voices<Facet>::zeros(num_voices);
        for (std::size_t i = 0; i < num_voices; ++i) {
            if (Trigger::contains_pulse_on(trigger[i])) {
                output[i].extend(m_random_handlers[i].process(chord_sizes[i]).as_type<Facet>());
            }
        }

        m_current_value = std::move(output);
        return m_current_value;
    }

private:
    std::size_t get_voice_count() {
        return voice_count(m_trigger.voice_count()
                           , m_chord_size.voice_count()
                           , m_num_quantization_steps.voice_count()
                           , m_weights.voice_count());
    }


    void update_parameters(std::size_t num_voices, bool resized) {
        auto mode = m_mode.process().first_or(RandomHandler::DEFAULT_MODE);
        auto reps = m_repetition_strategy.process().first_or(RandomHandler::DEFAULT_REPETITIONS);
        auto quantize = m_quantize.process().first_or(RandomHandler::DEFAULT_USE_QUANTIZATION);
        auto steps = m_num_quantization_steps.process(num_voices).firsts<std::size_t>();
        auto brownian_step = m_max_brownian_step.process().first_or(RandomHandler::DEFAULT_MAX_BROWNIAN_STEP);
        auto weights = m_weights.process(num_voices).as_type<double>();

        m_random_handlers.set(&RandomHandler::set_mode, mode);
        m_random_handlers.set(&RandomHandler::set_repetition_strategy, reps);
        m_random_handlers.set(&RandomHandler::set_quantization_steps, quantization_steps(quantize, steps));
        m_random_handlers.set(&RandomHandler::set_max_brownian_step, brownian_step);

        for (std::size_t i = 0; i < num_voices; ++i) {
            m_random_handlers[i].set_weights(weights[i]);
        }
    }


    static Voice<std::optional<std::size_t>> quantization_steps(bool quantize
                                                                , Voice<std::optional<std::size_t>> num_steps) {
        return num_steps.map([&quantize](const std::optional<std::size_t>& s) {
            return quantization_steps(quantize, s);
        });
    }


    static std::optional<std::size_t> quantization_steps(bool quantize, std::optional<std::size_t> num_steps) {
        if (!quantize) {
            return std::nullopt;
        }
        return num_steps;
    }


    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_mode;
    Socket<Facet>& m_repetition_strategy;
    Socket<Facet>& m_chord_size;
    Socket<Facet>& m_quantize;
    Socket<Facet>& m_num_quantization_steps;
    Socket<Facet>& m_max_brownian_step;
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

    Variable<Facet, bool> quantize{Keys::QUANTIZE, ph, RandomHandler::DEFAULT_USE_QUANTIZATION};
    Sequence<Facet, std::size_t> num_quantization_steps{Keys::NUM_QUANTIZATION_STEPS
                                                        , ph
                                                        , Voices<std::size_t>::singular(
                                                            RandomHandler::DEFAULT_NUM_QUANTIZATION_STEPS)
    };
    Variable<Facet, double> max_brownian_step{Keys::MAX_BROWNIAN_STEP, ph, RandomHandler::DEFAULT_MAX_BROWNIAN_STEP};

    Sequence<Facet, FloatType> weights{Keys::WEIGHTS, ph};

    Sequence<Facet, bool> enabled{param::properties::enabled, ph, Voices<bool>::singular(true)};
    Variable<Facet, std::size_t> num_voices{param::properties::num_voices, ph, 0};

    RandomNode random{Keys::CLASS_NAME
                      , ph
                      , &trigger
                      , &mode
                      , &repetition_strategy
                      , &chord_size
                      , &quantize
                      , &num_quantization_steps
                      , &max_brownian_step
                      , &weights
                      , &enabled
                      , &num_voices
    };
};
}

#endif //SERIALIST_RANDOM_NODE_H
