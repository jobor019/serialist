

#ifndef SERIALIST_LOOPER_SELECTOR_H
#define SERIALIST_LOOPER_SELECTOR_H

#include <vector>
#include <cmath>

#include "core/param/parameter_policy.h"
#include "core/generative.h"
#include "core/algo/time/transport.h"
#include "core/algo/random.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "core/algo/voice/multi_voiced.h"
#include "core/algo/time/trigger.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "core/generatives/sequence.h"
#include "core/generatives/variable.h"

#include <iostream>
#include <variant>
#include <vector>

class SelectionStrategy {
public:
    struct All {
    };
    struct None {
    };
    struct Nth {
        Vec<long> indices;
    };
    struct Randomize {
        std::size_t n;
    };
    struct Probabilistic { /* TODO */ };

    using StrategyType = std::variant<All, None, Nth, Randomize, Probabilistic>;

    SelectionStrategy() = default;


    explicit SelectionStrategy(const StrategyType& selectedStrategy) : strategy(selectedStrategy) {}


    static SelectionStrategy parse(const std::optional<Facet>& strategy_type
                                   , const Voice<Facet>& nth_indices
                                   , const std::optional<Facet>& num_random) {
        if (!strategy_type) {
            return {};
        }

        auto strategy_index = utils::double2index(strategy_type->get(), count());
        auto indices = nth_indices.as_type<long>();
        std::size_t n_rand = num_random.has_value() ? static_cast<std::size_t>(num_random.value()) : 0;
        return parse(strategy_index, indices, n_rand);
    }


    static SelectionStrategy parse(std::size_t index
                                   , const Vec<long>& nth_indices
                                   , std::size_t num_random) {
        if (index == 0) {
            return SelectionStrategy(All{});
        } else if (index == 1) {
            return SelectionStrategy(None{});
        } else if (index == 2) {
            return SelectionStrategy(Nth{nth_indices});
        } else {
            return SelectionStrategy(Randomize{num_random});
        } // TODO: Probabilistic
    }


    static std::size_t count() {
        return std::variant_size_v<StrategyType>;
    }


    template<typename T>
    bool is() const {
        return std::holds_alternative<T>(strategy);
    }


    template<typename T>
    T as() const {
        return std::get<T>(strategy);
    }

    // TODO: Serialize/deserialize

private:
    StrategyType strategy = All{};
};


// ==============================================================================================

template<typename T>
class Selector {
public:
    explicit Selector(unsigned int seed = 0) : m_rng(seed) {}


    Voice<T> process(const Voice<T>& chord, const SelectionStrategy& strategy) {
        if (chord.empty()) {
            return {};
        }

        return match(strategy, chord);
    }


private:
    Voice<T> match(const SelectionStrategy& strategy, const Voice<T>& chord) {
        if (strategy.is<SelectionStrategy::All>()) {
            return chord;
        } else if (strategy.is<SelectionStrategy::None>()) {
            return {};
        } else if (strategy.is<SelectionStrategy::Nth>()) {
            return nth(chord, strategy.as<SelectionStrategy::Nth>().indices);
        } else if (strategy.is<SelectionStrategy::Randomize>()) {
            return random(chord, strategy.as<SelectionStrategy::Randomize>().n);
        } else if (strategy.is<SelectionStrategy::Probabilistic>()) {
            return {};  // TODO
        } else {
            throw std::domain_error("Invalid selector type detected");
        }
    }


    static Voice<T> nth(const Voice<T>& chord, const Vec<long>& indices) {
        auto size = static_cast<long>(chord.size());
        auto adjusted_indices = indices.cloned().clip_remove(-(size - 1), size - 1);

        if (adjusted_indices.empty()) {
            return {};
        }

        return chord[adjusted_indices];
    }


    Voice<T> random(const Voice<T>& chord, std::size_t n) {
        auto num_elements = utils::clip(n, {0}, {chord.size() - 1});
        return m_rng.choices(chord, num_elements);
    }


    Random m_rng;
};


// ==============================================================================================

class SelectorAdapter : public StaticNode<SelectionStrategy> {
public:
    static const inline std::string STRATEGY = "strategy";
    static const inline std::string INDICES = "indices";
    static const inline std::string NUM_RANDOM = "num_random";

    static const inline std::string CLASS_NAME = "selection_adapter";


    SelectorAdapter(const std::string& id
                    , ParameterHandler& parent
                    , Node<Facet>* strategy
                    , Node<Facet>* nth_indices
                    , Node<Facet>* num_random)
            : StaticNode<SelectionStrategy>(id, parent, CLASS_NAME)
              , m_strategy(add_socket(STRATEGY, strategy))
              , m_nth_indices(add_socket(INDICES, nth_indices))
              , m_num_random(add_socket(NUM_RANDOM, num_random)) {}


    Voices<SelectionStrategy> process() {
        if (!m_strategy.has_changed() && !m_nth_indices.has_changed() && !m_num_random.has_changed()) {
            return m_current_value;
        }

        auto strategy = m_strategy.process();
        auto nth_indices = m_nth_indices.process();
        auto num_random = m_num_random.process();

        auto num_voices = std::max({strategy.size(), nth_indices.size(), num_random.size()});

        auto strategies = strategy.adapted_to(num_voices).firsts();
        nth_indices.adapted_to(num_voices);
        auto num_randoms = num_random.adapted_to(num_voices).firsts();

        auto output = Voices<SelectionStrategy>::zeros(num_voices);
        for (std::size_t i = 0; i < num_voices; ++i) {
            output[i].append(SelectionStrategy::parse(strategies[i], nth_indices[i], num_randoms[i]));
        }

        m_current_value = std::move(output);
        return m_current_value;
    }


private:
    Socket<Facet>& m_strategy;
    Socket<Facet>& m_nth_indices;
    Socket<Facet>& m_num_random;

    Voices<SelectionStrategy> m_current_value = Voices<SelectionStrategy>::empty_like();
};


// ==============================================================================================


struct SelectorKeys {
    static const inline std::string TRIGGER = "trigger";
    static const inline std::string MATERIAL = "material";
    static const inline std::string STRATEGY = "strategy";

    static const inline std::string CLASS_NAME = "selector";
};


// ==============================================================================================

template<typename T>
class SelectorNode : public NodeBase<T> {
public:
    SelectorNode(const std::string& id
                 , ParameterHandler parent
                 , Node<Trigger>* trigger
                 , Node<T>* material
                 , Node<SelectionStrategy>* strategy
                 , Node<Facet>* enabled
                 , Node<Facet>* num_voices)
            : NodeBase<T>(id, parent, enabled, num_voices, SelectorKeys::CLASS_NAME)
              , m_trigger(NodeBase<T>::add_socket(SelectorKeys::TRIGGER, trigger))
              , m_material(NodeBase<T>::add_socket(SelectorKeys::MATERIAL, material))
              , m_strategy(NodeBase<T>::add_socket(SelectorKeys::STRATEGY, strategy)) {}


    Voices<T> process() {
        if (auto t = NodeBase<T>::pop_time(); !t) {
            return m_current_value;
        }

        if (!NodeBase<T>::is_enabled() || !m_trigger.is_connected()) {
            m_current_value = Voices<T>::empty_like();
            return m_current_value;
        }

        auto trigger = m_trigger.process();

        if (trigger.is_empty_like()) {
            return m_current_value;
        }

        auto material = m_material.process();
        if (material.is_empty_like()) {
            m_current_value = Voices<T>::empty_like();
            return m_current_value;
        }

        auto strategy = m_strategy.process();

        auto num_voices = NodeBase<T>::voice_count(trigger.size(), material.size(), strategy.size());
        if (num_voices != m_selectors.size()) {
            m_selectors.resize(num_voices);
        }

        auto triggers = trigger.adapted_to(num_voices);
        auto materials = material.adapted_to(num_voices);
        auto strategies = strategy.adapted_to(num_voices).firsts_or(SelectionStrategy{});

        auto output = Voices<T>::zeros(num_voices);
        for (std::size_t i = 0; i < trigger.size(); ++i) {
            if (triggers[i].contains(Trigger::pulse_on)) {
                output[i] = m_selectors[i].process(materials[i], strategies[i]);
            }
        }

        m_current_value = std::move(output);
        return m_current_value;
    }


private:
    Socket<Trigger>& m_trigger;
    Socket<T>& m_material;
    Socket<SelectionStrategy>& m_strategy;

    MultiVoiced<Selector<T>, T> m_selectors;

    Voices<T> m_current_value = Voices<T>::empty_like();
};


// ==============================================================================================

template<typename OutputType, typename StoredType>
struct SelectorWrapper {
    ParameterHandler parameter_handler;

    Sequence<Trigger> trigger{SelectorKeys::TRIGGER, parameter_handler};
    Sequence<OutputType, StoredType> material{SelectorKeys::MATERIAL, parameter_handler};
    Sequence<SelectionStrategy> strategy{SelectorKeys::STRATEGY, parameter_handler};

    Sequence<Facet, bool> enabled{ParameterKeys::ENABLED, parameter_handler, true};
    Variable<Facet, std::size_t> num_voices{ParameterKeys::NUM_VOICES, parameter_handler, 1};

    SelectorNode<OutputType> selector{SelectorKeys::CLASS_NAME, parameter_handler, &trigger, &material
                                      , &strategy, &enabled, &num_voices};

};


#endif //SERIALIST_LOOPER_SELECTOR_H
