#ifndef SERIALIST_INTERPOLATOR_H
#define SERIALIST_INTERPOLATOR_H

#include <optional>
#include "core/collections/voices.h"
#include "core/types/facet.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "core/types/trigger.h"
#include "sequence.h"
#include "variable.h"
#include "types/index.h"


namespace serialist {

template<typename T>
class Interpolator {
public:
    using Mode = Index::Strategy;
    using IndexType = Index::IndexType;

    static constexpr auto DEFAULT_MODE = Mode::mod;
    static constexpr double DEFAULT_OCTAVE = 12.0; // TODO: This will obviously not work for non-numeric types T
    static constexpr bool DEFAULT_USES_INDEX = false;


    static Voice<T> process(const Index& index, const Voices<T>& corpus, Mode mode, const std::optional<T>& octave) {
        if (corpus.is_empty_like()) {
            return {};
        }

        if (mode == Mode::cont) {
            if (octave.has_value()) {
                return Index::apply_octave(index, corpus, *octave);
            }
            return corpus[index.get_mod(corpus.size())];
        }

        if (auto bounded_index = index.get(corpus.size(), mode)) {
            return corpus[*bounded_index];
        }

        // Mode::pass with index outside bounds
        return {};
    }


    static Voice<T> process(double cursor, const Voices<T>& corpus, Mode mode, const std::optional<T>& octave) {
        return process(Index::from_phase_like(cursor, corpus.size()), corpus, mode, octave);
    }
};


// ==============================================================================================

template<typename T>
class InterpolatorNode : public NodeBase<T> {
public:
    struct Keys {
        static const inline std::string CURSOR = "cursor";
        static const inline std::string CORPUS = "corpus";
        static const inline std::string MODE = "mode";
        static const inline std::string OCTAVE = "octave";
        static const inline std::string USES_INDEX = "uses_index";

        static const inline std::string CLASS_NAME = "interpolator";
    };


    InterpolatorNode(const std::string& id
                     , ParameterHandler& parent
                     , Node<Trigger>* trigger
                     , Node<Facet>* cursor
                     , Node<T>* corpus
                     , Node<Facet>* mode
                     , Node<Facet>* octave
                     , Node<Facet>* uses_index
                     , Node<Facet>* enabled
                     , Node<Facet>* num_voices)
        : NodeBase<T>(id, parent, enabled, num_voices, Keys::CLASS_NAME)
        , m_trigger(NodeBase<T>::add_socket(param::properties::trigger, trigger))
        , m_cursor(NodeBase<T>::add_socket(Keys::CURSOR, cursor))
        , m_corpus(NodeBase<T>::add_socket(Keys::CORPUS, corpus))
        , m_mode(NodeBase<T>::add_socket(Keys::MODE, mode))
        , m_octave(NodeBase<T>::add_socket(Keys::OCTAVE, octave))
        , m_uses_index(NodeBase<T>::add_socket(Keys::USES_INDEX, uses_index)) {}


    Voices<T> process() override {
        if (auto t = NodeBase<T>::pop_time(); !t) {
            return m_current_value;
        }

        if (disabled()) {
            m_current_value = Voices<T>::empty_like();
            return m_current_value;
        }

        auto trigger = m_trigger.process();

        if (trigger.is_empty_like()) {
            return m_current_value;
        }

        auto cursor = m_cursor.process();
        auto mode = m_mode.process();
        auto octave = m_octave.process();
        auto use_index = m_uses_index.process().first_or(Interpolator<T>::DEFAULT_USES_INDEX);

        auto num_voices = NodeBase<T>::voice_count(trigger.size(), cursor.size(), mode.size(), octave.size());

        auto triggers = trigger.adapted_to(num_voices);
        auto cursors = cursor.adapted_to(num_voices).firsts();
        auto modes = mode.adapted_to(num_voices).firsts_or(Interpolator<T>::DEFAULT_MODE);
        auto octaves = octave.adapted_to(num_voices).template firsts<T>();

        auto corpus = m_corpus.process();

        m_current_value.adapted_to(num_voices);
        for (std::size_t i = 0; i < trigger.size(); ++i) {
            if (Trigger::contains_pulse_on(triggers[i]) && cursors[i].has_value()) {
                if (use_index) {
                    m_current_value[i] = Interpolator<T>::process(
                        Index::from_index_facet(*cursors[i])
                        , corpus
                        , modes[i]
                        , octaves[i]
                    );

                } else {
                    m_current_value[i] = Interpolator<T>::process(
                        static_cast<double>(*cursors[i])
                        , corpus
                        , modes[i]
                        , octaves[i]
                    );
                }
            }
        }

        return m_current_value;
    }

private:
    bool disabled() {
        return !NodeBase<T>::is_enabled()
               || !m_trigger.is_connected()
               || !m_corpus.is_connected()
               || !m_cursor.is_connected();
    }


    Socket<Trigger>& m_trigger;
    Socket<Facet>& m_cursor;
    Socket<T>& m_corpus;
    Socket<Facet>& m_mode;
    Socket<Facet>& m_octave;
    Socket<Facet>& m_uses_index;

    Voices<T> m_current_value = Voices<T>::empty_like();
};


// ==============================================================================================

template<typename OutputType, typename StoredType, typename FloatType = float>
struct InterpolatorWrapper {
    using Keys = typename InterpolatorNode<OutputType>::Keys;
    using Mode = typename Interpolator<OutputType>::Mode;
    using InterpolatorT = Interpolator<OutputType>;

    ParameterHandler ph;

    Sequence<Trigger> trigger{param::properties::trigger, ph, Trigger::pulse_on()};
    Sequence<Facet, FloatType> cursor{Keys::CURSOR, ph};
    Sequence<OutputType, StoredType> corpus{Keys::CORPUS, ph};
    Sequence<Facet, Mode> mode{Keys::MODE, ph, Voices<Mode>::singular(InterpolatorT::DEFAULT_MODE)};
    Sequence<Facet, FloatType> octave{Keys::OCTAVE, ph, Voices<double>::singular(InterpolatorT::DEFAULT_OCTAVE)};
    Variable<Facet, bool> uses_index{Keys::USES_INDEX, ph, InterpolatorT::DEFAULT_USES_INDEX};

    Sequence<Facet, bool> enabled{param::properties::enabled, ph, true};
    Variable<Facet, std::size_t> num_voices{param::properties::num_voices, ph, 1};

    InterpolatorNode<OutputType> interpolator{Keys::CLASS_NAME
        , ph
        , &trigger
        , &cursor
        , &corpus
        , &mode
        , &octave
        , &uses_index
        , &enabled
        , &num_voices
    };
};

template<typename FloatType = double>
using InterpolatorDoubleWrapper = InterpolatorWrapper<Facet, double, FloatType>;

template<typename FloatType = double>
using InterpolatorIntWrapper = InterpolatorWrapper<Facet, int, FloatType>;

template<typename FloatType = double>
using InterpolatorStringWrapper = InterpolatorWrapper<std::string, std::string, FloatType>;


} // namespace serialist

#endif //SERIALIST_INTERPOLATOR_H
