#ifndef SERIALIST_PATTERNIZER_H
#define SERIALIST_PATTERNIZER_H

#include "sequence.h"
#include "variable.h"
#include "algo/random.h"
#include "policies/parameter_policy.h"
#include "policies/socket_policy.h"
#include "stereotypes/base_stereotypes.h"
#include "types/index.h"


namespace serialist {
// ==============================================================================================

// TODO: REMOVE OR USE
// // Note: namespace to avoid overly verbose templated types like Socket<template Patternizer::Mode>
// namespace patternizer {
//
// }

template<typename T>
class Patternizer {
public:
    explicit Patternizer(std::optional<unsigned int> seed = std::nullopt) : m_rnd(seed) {}

    enum class Mode {
        from_bottom, from_top, random, /* TODO: probabilistic */
    };

    static constexpr Mode DEFAULT_MODE = Mode::from_bottom;
    static constexpr auto DEFAULT_STRATEGY = Index::Strategy::mod;
    static constexpr bool DEFAULT_CURSOR_IS_INDEX = true;


    Voice<T> process(const Voice<T>& v
                     , const Voice<Index>& indices
                     , Mode mode
                     , std::optional<T> octave
                     , Index::Strategy strategy) {
        if (v.empty())
            return v;

        if (mode == Mode::random) {
            return random(v, indices.first());
        }
        return select(v, indices, octave, strategy, mode == Mode::from_top);
    }


    Voice<T> process(const Voice<T>& v
                     , const Voice<double>& phase_like
                     , Mode mode
                     , std::optional<T> octave
                     , Index::Strategy strategy) {
        return process(v
                       , phase_like.as_type<Index>([&v](double p) { return Index::from_phase_like(p, v.size()); })
                       , mode
                       , octave
                       , strategy
        );
    }

private:
    Voice<T> select(const Voice<T>& v
                    , const Voice<Index>& indices
                    , std::optional<T> octave
                    , Index::Strategy strategy
                    , bool invert) {
        assert(!v.empty());

        auto result = Voice<T>::allocated(indices.size());
        for (const Index& i : indices) {
            if (auto selected = select_single(v, i, octave, strategy, invert)) {
                result.append(*selected);
            }
        }

        return result;
    }


    Voice<T> random(const Voice<T>& v, std::optional<Index> count) {
        if (!count || v.empty() || *count == 0)
            return {};

        return m_rnd.choices(v, count->get_raw(), true);
    }


    std::optional<T> select_single(const Voice<T>& v
                           , const Index& index
                           , std::optional<T> octave
                           , Index::Strategy strategy
                           , bool invert) {
        if (strategy == Index::Strategy::cont && octave) {
            if constexpr (std::is_arithmetic_v<T>) {
                auto num_octaves = index.get_octave(v.size());
                auto i = index.get_mod(v.size(), invert);

                return v[i] + *octave * num_octaves;
            } else {
                // Non-arithmetic types do not support octaves
                return v[index.get_mod(v.size(), invert)];
            }
        }

        if (auto i = index.get(v.size(), strategy, invert)) {
            return v[*i];
        }
        return std::nullopt;

    }


    Random m_rnd;
};


// ==============================================================================================

template<typename T>
class PatternizerNode : public NodeBase<T> {
public:
    struct Keys {
        static const inline std::string CHORD = "chord";
        static const inline std::string CURSOR = "cursor";
        static const inline std::string MODE = "mode";
        static const inline std::string STRATEGY = "strategy";
        static const inline std::string CURSOR_IS_INDEX = "cursor_is_index";
        static const inline std::string OCTAVE = "octave";

        static const inline std::string CLASS_NAME = "patternizer";
    };


    PatternizerNode(const std::string& id
                    , ParameterHandler& parent
                    , Node<Trigger>* trigger = nullptr
                    , Node<T>* chord = nullptr
                    , Node<Facet>* cursor = nullptr
                    , Node<Facet>* mode = nullptr
                    , Node<Facet>* strategy = nullptr
                    , Node<Facet>* cursor_is_index = nullptr
                    , Node<T>* octave = nullptr
                    , Node<Facet>* enabled = nullptr
                    , Node<Facet>* num_voices = nullptr)
        : NodeBase<T>(id, parent, enabled, num_voices, Keys::CLASS_NAME)
        , m_trigger(NodeBase<T>::add_socket(param::properties::trigger, trigger))
        , m_chord(NodeBase<T>::add_socket(Keys::CHORD, chord))
        , m_cursor(NodeBase<T>::add_socket(Keys::CURSOR, cursor))
        , m_mode(NodeBase<T>::add_socket(Keys::MODE, mode))
        , m_strategy(NodeBase<T>::add_socket(Keys::STRATEGY, strategy))
        , m_cursor_is_index(NodeBase<T>::add_socket(Keys::CURSOR_IS_INDEX, cursor_is_index))
        , m_octave(NodeBase<T>::add_socket(Keys::OCTAVE, octave)) {}


    Voices<T> process() {
        if (auto t = NodeBase<T>::pop_time(); !t) {
            return m_current_value;
        }

        if (!NodeBase<T>::is_enabled() || !m_trigger.is_connected() || !m_chord.is_connected() || !m_cursor.
            is_connected()) {
            m_current_value = Voices<T>::empty_like();
            return m_current_value;
        }

        auto trigger = m_trigger.process();
        if (trigger.is_empty_like()) { return m_current_value; }

        auto chord = m_chord.process();
        auto cursor = m_cursor.process();
        auto mode = m_mode.process();
        auto strategy = m_strategy.process();
        auto cursor_is_index = m_cursor_is_index.process();
        auto octave = m_octave.process();

        auto num_voices = NodeBase<T>::voice_count(trigger.size(), chord.size(), cursor.size(), mode.size(), octave.size());

        if (num_voices != m_patternizers.size())
            m_patternizers.resize(num_voices);

        auto triggers = trigger.adapted_to(num_voices);
        auto chords = chord.adapted_to(num_voices);

        // Note: Patternizer supports (expects) multiple cursors. We should not use firsts() here.
        auto cursors = cursor.adapted_to(num_voices);

        auto modes = mode.adapted_to(num_voices).firsts_or(Patternizer<T>::DEFAULT_MODE);
        auto octaves = octave.adapted_to(num_voices).firsts();

        auto current_strategy = mode.first_or(Patternizer<T>::DEFAULT_STRATEGY);
        bool is_index = cursor_is_index.first_or(Patternizer<T>::DEFAULT_CURSOR_IS_INDEX);

        m_current_value.adapted_to(num_voices);
        for (size_t i = 0; i < num_voices; ++i) {
            if (Trigger::contains_pulse_on(triggers[i])) {
                if (is_index) {
                    m_current_value[i] = m_patternizers[i].process(
                        chords[i]
                        , cursors[i].template as_type<Index>([](const Facet& f) { return Index::from_index_facet(f); })
                        , modes[i]
                        , octaves[i]
                        , current_strategy
                    );
                } else {
                    m_current_value[i] = m_patternizers[i].process(
                        chords[i]
                        , cursors[i].template as_type<double>()
                        , modes[i]
                        , octaves[i]
                        , current_strategy
                    );
                }
            }
        }

        return m_current_value;
    }

private:
    Socket<Trigger>& m_trigger;
    Socket<T>& m_chord;
    Socket<Facet>& m_cursor;

    Socket<Facet>& m_mode;
    Socket<Facet>& m_strategy;
    Socket<Facet>& m_cursor_is_index;
    Socket<T>& m_octave;

    MultiVoiced<Patternizer<T>, T> m_patternizers;

    Voices<T> m_current_value = Voices<T>::empty_like();
};

// ==============================================================================================

template<typename OutputType, typename StoredType, typename FloatType = double>
struct PatternizerWrapper {
    using Keys = typename PatternizerNode<OutputType>::Keys;
    using Mode = typename Patternizer<OutputType>::Mode;
    using PatternizerT = Patternizer<OutputType>;


    ParameterHandler ph;

    Sequence<Trigger> trigger{param::properties::trigger, ph};
    Sequence<OutputType, StoredType> chord{Keys::CHORD, ph};
    Sequence<Facet, FloatType> cursor{Keys::CURSOR, ph};
    Sequence<Facet, Mode> mode{Keys::MODE, ph, Voices<Mode>::singular(PatternizerT::DEFAULT_MODE)};
    Variable<Facet, Index::Strategy> strategy{Keys::STRATEGY, ph, PatternizerT::DEFAULT_STRATEGY};
    Variable<Facet, bool> cursor_is_index{Keys::CURSOR_IS_INDEX, ph, PatternizerT::DEFAULT_CURSOR_IS_INDEX};
    Sequence<OutputType, StoredType> octave{Keys::OCTAVE, ph};

    Sequence<Facet, bool> enabled{param::properties::enabled, ph, Voices<bool>::singular(true)};
    Variable<Facet, std::size_t> num_voices{param::properties::num_voices, ph, 0};

    PatternizerNode<OutputType> patternizer_mode{Keys::CLASS_NAME
                                                 , ph
                                                 , &trigger
                                                 , &chord
                                                 , &cursor
                                                 , &mode
                                                 , &strategy
                                                 , &cursor_is_index
                                                 , &octave
                                                 , &enabled
                                                 , &num_voices
    };
};

template<typename FloatType = double>
using PatternizerDoubleWrapper = PatternizerWrapper<Facet, double, FloatType>;

template<typename FloatType = double>
using PatternizerIntWrapper = PatternizerWrapper<Facet, int, FloatType>;

template<typename FloatType = double>
using PatternizerStringWrapper = PatternizerWrapper<std::string, std::string, FloatType>;
}

#endif //SERIALIST_PATTERNIZER_H
