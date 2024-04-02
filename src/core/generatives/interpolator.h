
#ifndef SERIALISTLOOPER_INTERPOLATOR_H
#define SERIALISTLOOPER_INTERPOLATOR_H

#include <sstream>
#include <optional>
#include "core/collections/voices.h"
#include "core/algo/facet.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "core/algo/time/trigger.h"
#include "core/utility/math.h"
#include "sequence.h"
#include "variable.h"


// TODO: Serialization. Should probably generalize this into a Serializer class with a number of different overloads
//std::string to_string() const {
//    std::ostringstream oss;
//    oss << static_cast<int>(m_type) << SEPARATOR << m_pivot;
//    return oss.str();
//}
//
//
//static SelectionStrategy from_string(const std::string& s) {
//    std::istringstream iss(s);
//
//    int type;
//    float pivot;
//    char dump;
//
//    iss >> type >> dump >> pivot;
//
//    return SelectionStrategy(static_cast<Mode>(type), pivot);
//}


// ==============================================================================================

template<typename T>
class Interpolator {
public:
    static constexpr double epsilon = 1e-6;

    struct Continue {
        T pivot;
        // TODO: Implement to_string and from_string (utils::is_serializable)
    };
    struct Modulo {
        // TODO: Implement to_string and from_string (utils::is_serializable)
    };
    struct Clip {
        // TODO: Implement to_string and from_string (utils::is_serializable)
    };
    struct Pass {
        // TODO: Implement to_string and from_string (utils::is_serializable)
    };

    using Strategy = std::variant<Continue, Modulo, Clip, Pass>;


    static Strategy default_strategy() {
        return Clip();
    }


    static Voice<T> process(double cursor, const Voices<T>& corpus, const Strategy& strategy) {
        if (corpus.is_empty_like()) {
            return {};
        }

        return std::visit([&cursor, &corpus](const auto& s) {
            return match(s, cursor, corpus);
        }, strategy);
    }


private:
    static Voice<T> match(const Strategy& strategy, double cursor, const Voices<T>& corpus) {
        if (std::holds_alternative<Continue>(strategy)) {
            if constexpr (!std::is_arithmetic_v<T>) {
                return modulo(cursor, corpus);
            } else {
                return continuation(cursor, corpus, std::get<Continue>(strategy).pivot);
            }
        } else if (std::holds_alternative<Modulo>(strategy)) {
            return modulo(cursor, corpus);
        } else if (std::holds_alternative<Clip>(strategy)) {
            return clip(cursor, corpus);
        } else if (std::holds_alternative<Pass>(strategy)) {
            return pass(cursor, corpus);
        } else {
            throw std::domain_error("Invalid strategy type");
        }
    }


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    static Voice<T> continuation(double cursor, const Voices<T>& corpus, const T& pivot) {
        auto index = static_cast<std::size_t>(utils::modulo(index_of(cursor, corpus.size())
                                                            , static_cast<long>(corpus.size())));
        auto multiplier = static_cast<T>(std::floor(cursor / 1.0));
        std::cout << "continuation (cursor=" << cursor << ", index=" << index << ", multiplier=" << multiplier << ")" << std::endl;
        return corpus[index] + pivot * multiplier;
    }


    static Voice<T> modulo(double cursor, const Voices<T>& corpus) {
        auto index = static_cast<std::size_t>(utils::modulo(index_of(cursor, corpus.size())
                                                            , static_cast<long>(corpus.size())));
        std::cout << "modulo (cursor=" << cursor << ", index=" << index << ")" << std::endl;
        return corpus[index];
    }


    static Voice<T> clip(double cursor, const Voices<T>& corpus) {
        auto index = static_cast<std::size_t>(utils::clip(index_of(cursor, corpus.size())
                                                          , {0L}, {static_cast<long>(corpus.size() - 1)}));
        std::cout << "clip (cursor=" << cursor << ", index=" << index << ")" << std::endl;
        return corpus[index];
    }


    static Voice<T> pass(double cursor, const Voices<T>& corpus) {
        auto index = index_of(cursor, corpus.size());

        if (index < 0 || index >= static_cast<long>(corpus.size())) {
            std::cout << "pass INVALID (cursor=" << cursor << ", index=" << index << ")" << std::endl;
            return {};
        }

        std::cout << "pass VALID (cursor=" << cursor << ", index=" << index << ")" << std::endl;
        return corpus[static_cast<std::size_t>(index)];
    }


    static long index_of(double position, std::size_t map_size) {
        // This should work correctly up to Mappings of size 67_108_864,
        //   assuming doubles of 8 bytes (tested up to 10_000)
        return static_cast<long>( std::floor(position * static_cast<double>(map_size) + epsilon));
    }
};


// ==============================================================================================

template<typename PivotType>
class InterpolationAdapter : public Node<typename Interpolator<PivotType>::Strategy> {
public:

    using Strategy = typename Interpolator<PivotType>::Strategy;

    static const inline std::string PIVOT = "pivot";
    static const inline std::string STRATEGY = "strategy";
    static const inline std::string CLASS_NAME = "interpolation_adapter";

    static constexpr std::size_t NUM_STRATEGIES = std::variant_size_v<Strategy>;


    InterpolationAdapter(const std::string& id
                         , ParameterHandler& parent
                         , Node<Facet>* strategy = nullptr
                         , Node<PivotType>* pivot = nullptr)
            : m_parameter_handler(id, parent)
              , m_socket_handler(m_parameter_handler)
              , m_strategy(m_socket_handler.create_socket(STRATEGY, strategy))
              , m_pivot(m_socket_handler.create_socket(PIVOT, pivot)) {
        m_parameter_handler.add_static_property(ParameterKeys::GENERATIVE_CLASS, CLASS_NAME);
    }


    std::vector<Generative*> get_connected() override {
        return m_socket_handler.get_connected();
    }


    ParameterHandler& get_parameter_handler() override {
        return m_parameter_handler;
    }


    void disconnect_if(Generative& connected_to) override {
        m_socket_handler.disconnect_if(connected_to);
    }


    Voices<Strategy> process() override {
        if (!m_strategy.has_changed() && !m_pivot.has_changed()) {
            return m_current_value;
        }

        auto strategy = m_strategy.process();
        auto pivot = m_pivot.process();

        auto num_voices = std::max(strategy.size(), pivot.size());

        auto strategies = strategy.adapted_to(num_voices).firsts();
        auto pivots = pivot.adapted_to(num_voices).firsts();

        auto output = Voices<typename Interpolator<PivotType>::Strategy>::zeros(num_voices);
        for (std::size_t i = 0; i < strategies.size(); ++i) {
            output[i].append(parse_strategy(strategies[i], pivots[i]));
        }

        m_current_value = std::move(output);

        return m_current_value;
    }


    static Strategy parse_strategy(const std::optional<Facet>& strategy_type
                                   , const std::optional<PivotType>& pivot) {
        if (!strategy_type) {
            // default
            return typename Interpolator<PivotType>::Clip();
        }

        auto index = utils::double2index(strategy_type->get(), NUM_STRATEGIES);

        return parse_strategy(index, pivot);
    }


    static Strategy parse_strategy(std::size_t index, const std::optional<PivotType>& pivot) {
        if (index == 0) {
            if (pivot) {
                return typename Interpolator<PivotType>::Continue{*pivot};
            }
            return typename Interpolator<PivotType>::Modulo();

        } else if (index == 1) {
            return typename Interpolator<PivotType>::Modulo();
        } else if (index == 2) {
            return typename Interpolator<PivotType>::Clip();
        } else {
            return typename Interpolator<PivotType>::Pass();
        }
    }

    static double index2double(std::size_t index) {
        return utils::index2double(index, NUM_STRATEGIES);
    }


private:
    ParameterHandler m_parameter_handler;
    SocketHandler m_socket_handler;

    Socket<Facet>& m_strategy;
    Socket<PivotType>& m_pivot;


    Voices<Strategy> m_current_value = Voices<Strategy>::empty_like();
};


// ==============================================================================================

struct InterpolatorKeys {
    static const inline std::string TRIGGER = "trigger";
    static const inline std::string CURSOR = "cursor";
    static const inline std::string CORPUS = "corpus";
    static const inline std::string STRATEGY = "strategy";

    static const inline std::string CLASS_NAME = "interpolator";
};


// ==============================================================================================

template<typename OutputType>
class InterpolatorNode : public NodeBase<OutputType> {
public:

    InterpolatorNode(const std::string& id
                     , ParameterHandler& parent
                     , Node<Trigger>* trigger
                     , Node<Facet>* cursor
                     , Node<OutputType>* corpus
                     , Node<typename Interpolator<OutputType>::Strategy>* strategy
                     , Node<Facet>* enabled
                     , Node<Facet>* num_voices)
            : NodeBase<OutputType>(id, parent, enabled, num_voices, InterpolatorKeys::CLASS_NAME)
              , m_trigger(NodeBase<OutputType>::add_socket(InterpolatorKeys::TRIGGER, trigger))
              , m_cursor(NodeBase<OutputType>::add_socket(InterpolatorKeys::CURSOR, cursor))
              , m_corpus(NodeBase<OutputType>::add_socket(InterpolatorKeys::CORPUS, corpus))
              , m_strategy(NodeBase<OutputType>::add_socket(InterpolatorKeys::STRATEGY, strategy)) {}


    Voices<OutputType> process() {
        if (auto t = NodeBase<OutputType>::pop_time(); !t) {
            return m_current_value;
        }

        if (!NodeBase<OutputType>::is_enabled() || !m_trigger.is_connected()) {
            m_current_value = Voices<OutputType>::empty_like();
            return m_current_value;
        }

        auto trigger = m_trigger.process();

        if (trigger.is_empty_like()) {
            return m_current_value;
        }

        auto cursor = m_cursor.process();
        auto strategy = m_strategy.process();

        auto num_voices = NodeBase<OutputType>::voice_count(trigger.size(), cursor.size(), strategy.size());

        auto triggers = trigger.adapted_to(num_voices);
        auto cursors = cursor.adapted_to(num_voices).firsts_or(0.0);
        auto strategies = strategy.adapted_to(num_voices).firsts_or(Interpolator<OutputType>::default_strategy());

        auto corpus = m_corpus.process();

        auto output = Voices<OutputType>::zeros(num_voices);
        for (std::size_t i = 0; i < trigger.size(); ++i) {
            if (Trigger::contains_pulse_on(triggers[i])) {
                output[i] = Interpolator<OutputType>::process(cursors[i], corpus, strategies[i]);
            }
        }

        m_current_value = std::move(output);
        return m_current_value;
    }


private:
    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_cursor;
    Socket<OutputType>& m_corpus;
    Socket<typename Interpolator<OutputType>::Strategy>& m_strategy;

    Voices<OutputType> m_current_value = Voices<OutputType>::empty_like();
};


// ==============================================================================================

template<typename OutputType, typename StoredType, typename FloatType = float>
struct InterpolatorWrapper {
    using Strategy = typename Interpolator<OutputType>::Strategy;

    ParameterHandler parameter_handler;

    Sequence<Trigger> trigger{InterpolatorKeys::TRIGGER, parameter_handler};
    Sequence<Facet, FloatType> cursor{InterpolatorKeys::CURSOR, parameter_handler};
    Sequence<OutputType, StoredType> corpus{InterpolatorKeys::CORPUS, parameter_handler};
    Sequence<Strategy> strategy{InterpolatorKeys::STRATEGY, parameter_handler};

    Sequence<Facet, bool> enabled{ParameterKeys::ENABLED, parameter_handler, true};
    Variable<Facet, std::size_t> num_voices{ParameterKeys::NUM_VOICES, parameter_handler, 1};

    InterpolatorNode<OutputType> interpolator{InterpolatorKeys::CLASS_NAME, parameter_handler
                                              , &trigger, &cursor, &corpus, &strategy, &enabled, &num_voices};
};


#endif //SERIALISTLOOPER_INTERPOLATOR_H
