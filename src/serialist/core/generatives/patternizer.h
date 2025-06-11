#ifndef SERIALIST_PATTERNIZER_H
#define SERIALIST_PATTERNIZER_H

#include "sequence.h"
#include "variable.h"
#include "policies/parameter_policy.h"
#include "policies/socket_policy.h"
#include "stereotypes/base_stereotypes.h"
#include "types/index.h"


namespace serialist {


template<typename T>
class Patternizer {
public:
    enum class Mode { mod, cont, clip, pass, negative };

    static constexpr auto DEFAULT_MODE = Mode::mod;
    static constexpr bool DEFAULT_PATTERN_USES_INDEX = false;
    static constexpr bool DEFAULT_INVERTED = false;


    Index::Strategy mode_to_strategy(Mode mode) {
        switch (mode) {
            case Mode::mod:
                return Index::Strategy::mod;
            case Mode::cont:
                return Index::Strategy::cont;
            case Mode::clip:
                return Index::Strategy::clip;
            case Mode::pass:
                return Index::Strategy::pass;
            default:
                throw std::runtime_error("Unsupported mode");
        }
    }


    Voice<T> process(const Voice<T>& v
                     , const Voice<Index>& indices
                     , Mode mode
                     , std::optional<T> octave
                     , bool invert) {
        if (v.empty())
            return v;

        if (mode == Mode::negative) {
            return select_negative(v, indices, invert);
        }

        return select(v, indices, mode, octave, invert);
    }


    Voice<T> process(const Voice<T>& v
                     , const Voice<double>& phase_like
                     , Mode mode
                     , std::optional<T> octave
                     , bool invert) {
        return process(v
                       , phase_like.as_type<Index>([&v](double p) { return Index::from_phase_like(p, v.size()); })
                       , mode
                       , octave
                       , invert
        );
    }

private:
    Voice<T> select(const Voice<T>& v
                    , const Voice<Index>& indices
                    , Mode mode
                    , std::optional<T> octave
                    , bool invert) {
        assert(!v.empty());
        assert(mode != Mode::negative);

        auto result = Voice<T>::allocated(indices.size());
        for (const Index& i : indices) {
            if (auto selected = select_single(v, i, mode, octave, invert)) {
                result.append(*selected);
            }
        }

        return result;
    }

    Voice<T> select_negative(const Voice<T>& v
                             , const Voice<Index>& indices
                             , bool invert) {
        Vec<std::size_t> indices_to_remove;
        for (const auto& index : indices) {
            if (auto i = index.get_pass(v.size(), invert)) {
                indices_to_remove.append(*i);
            }
        }

        Voice<T> result;
        for (std::size_t i = 0; i < v.size(); ++i) {
            if (!indices_to_remove.contains(i)) {
                result.append(v[i]);
            }
        }

        return result;
    }


    std::optional<T> select_single(const Voice<T>& v
                                   , const Index& index
                                   , Mode mode
                                   , std::optional<T> octave
                                   , bool invert) {
        if (mode == Mode::cont && octave) {
            return Index::apply_octave(index, v, *octave);
        }

        if (auto i = index.get(v.size(), mode_to_strategy(mode), invert)) {
            return v[*i];
        }
        return std::nullopt;

    }
};


// ==============================================================================================

template<typename T>
class PatternizerNode : public NodeBase<T> {
public:
    struct Keys {
        static const inline std::string CHORD = "chord";
        static const inline std::string PATTERN = "pattern";
        static const inline std::string MODE = "mode";
        static const inline std::string INVERSE_SELECTION = "inverse";
        static const inline std::string PATTERN_USES_INDEX = "pattern_uses_index";
        static const inline std::string OCTAVE = "octave";

        static const inline std::string CLASS_NAME = "patternizer";
    };


    PatternizerNode(const std::string& id
                    , ParameterHandler& parent
                    , Node<Trigger>* trigger = nullptr
                    , Node<T>* chord = nullptr
                    , Node<Facet>* pattern = nullptr
                    , Node<Facet>* mode = nullptr
                    , Node<Facet>* inverse_selection = nullptr
                    , Node<Facet>* pattern_uses_index = nullptr
                    , Node<T>* octave = nullptr
                    , Node<Facet>* enabled = nullptr
                    , Node<Facet>* num_voices = nullptr)
        : NodeBase<T>(id, parent, enabled, num_voices, Keys::CLASS_NAME)
        , m_trigger(NodeBase<T>::add_socket(param::properties::trigger, trigger))
        , m_chord(NodeBase<T>::add_socket(Keys::CHORD, chord))
        , m_pattern(NodeBase<T>::add_socket(Keys::PATTERN, pattern))
        , m_mode(NodeBase<T>::add_socket(Keys::MODE, mode))
        , m_inverse_selection(NodeBase<T>::add_socket(Keys::INVERSE_SELECTION, inverse_selection))
        , m_pattern_uses_index(NodeBase<T>::add_socket(Keys::PATTERN_USES_INDEX, pattern_uses_index))
        , m_octave(NodeBase<T>::add_socket(Keys::OCTAVE, octave)) {}


    Voices<T> process() {
        if (auto t = NodeBase<T>::pop_time(); !t) {
            return m_current_value;
        }

        if (!NodeBase<T>::is_enabled() || !m_trigger.is_connected() || !m_chord.is_connected() || !m_pattern.
            is_connected()) {
            m_current_value = Voices<T>::empty_like();
            return m_current_value;
        }

        auto trigger = m_trigger.process();
        if (trigger.is_empty_like()) { return m_current_value; }

        auto chord = m_chord.process();
        auto pattern = m_pattern.process();
        auto mode = m_mode.process();
        auto invert = m_inverse_selection.process();
        auto pattern_uses_index = m_pattern_uses_index.process();
        auto octave = m_octave.process();

        auto num_voices = NodeBase<T>::voice_count(trigger.size(), chord.size(), pattern.size(), mode.size()
                                                   , octave.size());

        if (num_voices != m_patternizers.size())
            m_patternizers.resize(num_voices);

        auto triggers = trigger.adapted_to(num_voices);
        auto chords = chord.adapted_to(num_voices);
        auto patterns = pattern.adapted_to(num_voices);

        auto modes = mode.adapted_to(num_voices).firsts_or(Patternizer<T>::DEFAULT_MODE);
        auto octaves = octave.adapted_to(num_voices).firsts();

        auto current_strategy = invert.first_or(Patternizer<T>::DEFAULT_INVERTED);
        bool is_index = pattern_uses_index.first_or(Patternizer<T>::DEFAULT_PATTERN_USES_INDEX);

        m_current_value.adapted_to(num_voices);
        for (size_t i = 0; i < num_voices; ++i) {
            if (Trigger::contains_pulse_on(triggers[i])) {
                if (is_index) {
                    m_current_value[i] = m_patternizers[i].process(
                        chords[i]
                        , patterns[i].template as_type<Index>([](const Facet& f) { return Index::from_index_facet(f); })
                        , modes[i]
                        , octaves[i]
                        , current_strategy
                    );

                } else {
                    m_current_value[i] = m_patternizers[i].process(
                        chords[i]
                        , patterns[i].template as_type<double>()
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
    Socket<Facet>& m_pattern;

    Socket<Facet>& m_mode;
    Socket<Facet>& m_inverse_selection; // true: select from top/reverse, false: select from bottom/forward
    Socket<Facet>& m_pattern_uses_index;
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
    Sequence<Facet, FloatType> pattern{Keys::PATTERN, ph};
    Sequence<Facet, Mode> mode{Keys::MODE, ph, Voices<Mode>::singular(PatternizerT::DEFAULT_MODE)};
    Variable<Facet, bool> inverse{Keys::INVERSE_SELECTION, ph, PatternizerT::DEFAULT_INVERTED};
    Variable<Facet, bool> pattern_uses_index{Keys::PATTERN_USES_INDEX, ph, PatternizerT::DEFAULT_PATTERN_USES_INDEX};
    Sequence<OutputType, StoredType> octave{Keys::OCTAVE, ph};

    Sequence<Facet, bool> enabled{param::properties::enabled, ph, Voices<bool>::singular(true)};
    Variable<Facet, std::size_t> num_voices{param::properties::num_voices, ph, 0};

    PatternizerNode<OutputType> patternizer_mode{Keys::CLASS_NAME
                                                 , ph
                                                 , &trigger
                                                 , &chord
                                                 , &pattern
                                                 , &mode
                                                 , &inverse
                                                 , &pattern_uses_index
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
