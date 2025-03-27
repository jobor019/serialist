#ifndef TESTUTILS_CONDITION_H
#define TESTUTILS_CONDITION_H

#include "event.h"
#include "results.h"
#include "algo/pitch/notes.h"
#include "param/string_serialization.h"

using namespace serialist;


namespace serialist::test {
// ==============================================================================================
// CONSTANTS
// ==============================================================================================

static constexpr double EPSILON = 1e-8;
static constexpr double EPSILON2 = 1e-15;


// ==============================================================================================
// UTILITIES
// ==============================================================================================

template<typename T>
Facet fcast(const T& t) {
    static_assert(utils::is_static_castable_v<T, Facet>, "Type T must be castable to Facet");
    return static_cast<Facet>(t);
}


template<typename T>
std::string serialize(const T& t) {
    return StringSerializer<T>::to_string(t);
}


template<typename T>
bool is_empty(const Voices<T>& v) { return v.is_empty_like(); }


template<typename T>
bool is_singular(const Voices<T>& v) { return v.size() == 1 && v[0].size() == 1; }


// Note: "maybe" indicate that the value is either singular or empty
template<typename T>
bool is_maybe_singular(const Voices<T>& v) { return is_empty(v) || is_singular(v); }


// ==============================================================================================


template<typename T>
class GenericCondition {
public:
    virtual ~GenericCondition() {}


    /**
     * @return std::nullopt if no comparison could be made (e.g. first value in a loop), otherwise the result
     * @throws test_error if the comparison fails
     */
    virtual std::optional<bool> matches_last(const Vec<StepResult<T> >& values) const = 0;

    /**
     * @param values values to compare
     * @param allow_no_comparison if false, will throw test_error if no comparison could be made
     *                            (e.g. change comparison on single-valued input)
     *
     * @return std::nullopt if comparison is successful, otherwise the index of the first non-failing step
     * @throws test_error if the comparison fails
     */
    virtual std::optional<std::size_t> matches_all(const Vec<StepResult<T> >& values, bool allow_no_comparison) const =
    0;

    /**
     * @param values values to compare
     * @param allow_no_comparison if false, will throw test_error if no comparison could be made
     *                            (e.g. change comparison on single-valued input)
     * @return index of the first matching step, otherwise std::nullopt if the comparison is **unsuccessful**
     * @throws test_error if the comparison fails
     */
    virtual std::optional<std::size_t> matches_any(const Vec<StepResult<T> >& values, bool allow_no_comparison) const =
    0;

    /** @return the offset(s) in relation to a given step that were used for comparison
     *          (typically 0 for value comparison or {0, -1} for change comparison) */
    virtual Vec<int> compared_step_offsets() const = 0;
};


// ==============================================================================================

template<typename T>
class GenericOutputComparison : public GenericCondition<T> {
public:
    std::optional<bool> matches_last(const Vec<StepResult<T> >& values) const final {
        if (values.empty()) {
            return std::nullopt;
        }

        return matches_internal(values[values.size() - 1]);
    }


    std::optional<std::size_t> matches_all(const Vec<StepResult<T> >& values, bool allow_no_comparison) const final {
        return matches_vector(values, allow_no_comparison, [this](const StepResult<T>& step) {
            return !matches_internal(step); // Fail if any element matches
        });
    }


    std::optional<std::size_t> matches_any(const Vec<StepResult<T> >& values, bool allow_no_comparison) const final {
        return matches_vector(values, allow_no_comparison, [this](const StepResult<T>& step) {
            return matches_internal(step); // Pass if any element matches
        });
    }


    Vec<int> compared_step_offsets() const override { return Vec<int>::singular(0); }

protected:
    /** @throws test_error if the comparison fails */
    virtual bool matches_internal(const StepResult<T>& current) const = 0;


    std::optional<std::size_t> matches_vector(const Vec<StepResult<T> >& values,
                                              bool allow_no_comparison,
                                              const std::function<bool(const StepResult<T>&)>& match_func) const {
        if (values.empty()) {
            if (allow_no_comparison) {
                return std::nullopt;
            }
            throw test_error("No input to compare");
        }

        for (std::size_t i = 0; i < values.size(); ++i) {
            if (match_func(values[i])) {
                return i;
            }
        }
        return std::nullopt;
    }
};


// ==============================================================================================

template<typename T>
class GenericChangeComparison : public GenericCondition<T> {
public:
    using ChangeFunc = std::function<bool(const StepResult<T>&, const StepResult<T>&)>;


    std::optional<bool> matches_last(const Vec<StepResult<T> >& values) const final {
        if (values.size() < 2) {
            return std::nullopt;
        }

        return matches_internal(values[values.size() - 2], values[values.size() - 1]);
    }


    std::optional<std::size_t> matches_all(const Vec<StepResult<T> >& values, bool allow_no_comparison) const final {
        return matches_vector(values, allow_no_comparison
                              , [this](const StepResult<T>& prev, const StepResult<T>& cur) {
                                  return !matches_internal(prev, cur); // Fail if any element matches
                              });
    }


    std::optional<std::size_t> matches_any(const Vec<StepResult<T> >& values, bool allow_no_comparison) const final {
        return matches_vector(values, allow_no_comparison
                              , [this](const StepResult<T>& prev, const StepResult<T>& cur) {
                                  return matches_internal(prev, cur); // Pass if any element matches
                              });
    }


    Vec<int> compared_step_offsets() const override { return Vec{-1, 0}; }

protected:
    std::optional<std::size_t> matches_vector(const Vec<StepResult<T> >& values,
                                              bool allow_no_comparison,
                                              const ChangeFunc& match_func) const {
        if (values.size() < 2) {
            if (allow_no_comparison) {
                return std::nullopt;
            }
            throw test_error("Not enough input to compare");
        }

        for (std::size_t i = 1; i < values.size(); ++i) {
            if (match_func(values[i - 1], values[i])) {
                return i;
            }
        }
        return std::nullopt;
    }


    /** @throws test_error if the comparison fails */
    virtual bool matches_internal(const StepResult<T>& previous, const StepResult<T>& current) const = 0;
};


// ==============================================================================================

template<typename T>
class EmptyComparison : public GenericOutputComparison<T> {
    bool matches_internal(const StepResult<T>& current) const override {
        return current.voices().is_empty_like();
    }
};


template<typename T>
class FixedSizeComparison : public GenericOutputComparison<T> {
public:
    explicit FixedSizeComparison(std::optional<std::size_t> voices_size, std::optional<std::size_t> voice_size)
    : m_voices_size(voices_size), m_voice_size(voice_size) {
        assert(!m_voices_size || m_voices_size > 0); // This is a user input error: voices cannot be empty
    }

private:
    bool matches_internal(const StepResult<T>& current) const override {
        const auto& voices = current.voices();

        if (m_voices_size && voices.size() != *m_voices_size) return false;

        if (m_voice_size) {
            for (const auto& voice : voices) {
                if (voice.size() != *m_voice_size) {
                    return false;
                }
            }
        }
        return true;
    }

    std::optional<std::size_t> m_voices_size;
    std::optional<std::size_t> m_voice_size;

};


// ==============================================================================================


template<typename T>
class ValueComparison : public GenericOutputComparison<T> {
public:
    explicit ValueComparison(const std::function<bool(const T&)>& f) : m_f(f) {}

private:
    bool matches_internal(const StepResult<T>& current) const final {
        const auto& v = current.voices();

        if (v.is_empty_like()) throw test_error("Empty result: " + current.to_string() + ")");
        if (!is_singular(v)) throw test_error("Non-singular result: " + current.to_string() + ")");

        return m_f(v[0][0]);
    }


    std::function<bool(const T&)> m_f;
};

template<typename T>
class VoiceComparison : public GenericOutputComparison<T> {
public:
    explicit VoiceComparison(const std::function<bool(const Voice<T>&)>& f) : m_f(f) {}

private:
    bool matches_internal(const StepResult<T>& current) const final {
        const auto& v = current.voices();

        if (v.is_empty_like()) throw test_error("Empty result: " + current.to_string() + ")");

        return m_f(v[0]);
    }


    std::function<bool(const Voice<T>&)> m_f;
};

template<typename T>
class VoicesComparison : public GenericOutputComparison<T> {
public:
    explicit VoicesComparison(const std::function<bool(const Voices<T>&)>& f) : m_f(f) {}

private:
    bool matches_internal(const StepResult<T>& current) const final {
        return m_f(current.voices());
    }


    std::function<bool(const Voices<T>&)> m_f;
};




// ==============================================================================================

template<typename T>
class ValueChangeComparison : public GenericChangeComparison<T> {
public:
    using FuncType = std::function<bool(const T&, const T&)>;

    explicit ValueChangeComparison(const FuncType& f) : m_f(f) {}

private:
    bool matches_internal(const StepResult<T>& previous, const StepResult<T>& current) const final {
        const auto& p = previous.voices();
        const auto& c = current.voices();

        if (p.is_empty_like()) throw test_error("Empty result: " + previous.to_string() + ")");
        if (c.is_empty_like()) throw test_error("Empty result: " + current.to_string() + ")");

        if (!is_singular(p)) throw test_error("Non-singular result: " + previous.to_string() + ")");
        if (!is_singular(c)) throw test_error("Non-singular result: " + current.to_string() + ")");

        return m_f(p[0][0], c[0][0]);
    }


    FuncType m_f;
};



// ==============================================================================================

enum class Anchor {
    before, after
};


template<typename T>
class RunnerCondition {
public:
    struct NumSteps {
        std::size_t index;
    };
    struct After {
        DomainTimePoint time;
    };
    struct Before {
        DomainTimePoint time;
    };
    struct CompareTrue {
        std::unique_ptr<GenericCondition<T> > condition;
    };
    struct CompareFalse {
        std::unique_ptr<GenericCondition<T> > condition;
    };

    using Condition = std::variant<NumSteps, After, Before, CompareTrue, CompareFalse>;

    explicit RunnerCondition(Condition condition) : m_condition(std::move(condition)) {}


    static RunnerCondition from_time_point(const DomainTimePoint& t, Anchor anchor_type) {
        if (anchor_type == Anchor::before) {
            return RunnerCondition(Before{t});
        } else {
            return RunnerCondition(After{t});
        }
    }


    static RunnerCondition from_target_index(std::size_t target_index) {
        return RunnerCondition(NumSteps{target_index});
    }


    static RunnerCondition from_generic_condition(std::unique_ptr<GenericCondition<T> > c, bool compare_true) {
        if (compare_true) {
            return RunnerCondition(CompareTrue{std::move(c)});
        } else {
            return RunnerCondition(CompareFalse{std::move(c)});
        }
    }


    /** Naively predicts number of steps until stop condition,
     *  under the assumption that the step size is constant and meter doesn't change */
    std::optional<std::size_t> predict_num_steps(const TimePoint& current_time
                                                 , std::size_t current_step_index
                                                 , const DomainDuration& step_size) const {
        return std::visit([&](const auto& c) -> std::optional<std::size_t> {
            using VariantType = std::decay_t<decltype(c)>;

            if constexpr (std::is_same_v<VariantType, NumSteps>) {
                return c.index - current_step_index;
            } else if constexpr (std::is_same_v<VariantType, After>) {
                return steps(current_time, c.time, step_size) + 1;
            } else if constexpr (std::is_same_v<VariantType, Before>) {
                return steps(current_time, c.time, step_size);
            } else {
                return std::nullopt; // Cannot predict number of steps for other conditions
            }
        }, m_condition);
    }


    bool matches(std::size_t step_index
                  , const TimePoint& t
                  , const TimePoint& t_next
                  , const Vec<StepResult<T> >& v) const {
        return std::visit([&](const auto& cond) -> bool {
            using VariantType = std::decay_t<decltype(cond)>;
            if constexpr (std::is_same_v<VariantType, NumSteps>) {
                // Cannot use step_index, since condition may be evaluated over multiple runs,
                // where each run will reset the index to 0
                return step_index >= cond.index;
            } else if constexpr (std::is_same_v<VariantType, After>) {
                return t >= cond.time;
            } else if constexpr (std::is_same_v<VariantType, Before>) {
                return t_next >= cond.time;
            } else if constexpr (std::is_same_v<VariantType, CompareTrue>) {
                return cond.condition->matches_last(v).value_or(false);
            } else if constexpr (std::is_same_v<VariantType, CompareFalse>) {
                return !cond.condition->matches_last(v).value_or(true);
            } else {
                throw test_error("Unsupported condition type");
            }
        }, m_condition);
    }


    template<typename U>
    const U& as() const { return std::get<U>(m_condition); }


    template<typename U>
    bool is() const { return std::holds_alternative<U>(m_condition); }

    bool depends_on_step_output() const {
        return std::holds_alternative<CompareTrue>(m_condition) || std::holds_alternative<CompareFalse>(m_condition);
    }

private:
    static std::size_t steps(const TimePoint& current, const DomainTimePoint& target, const DomainDuration& step_size) {
        auto distance = (target - current).as_type(step_size.get_type(), current.get_meter());
        return static_cast<std::size_t>(std::ceil(distance.get_value() / step_size.get_value()));
    }


    Condition m_condition;
};


// ==============================================================================================


struct NoteComparator {
    std::optional<NoteNumber> nn;
    std::optional<uint32_t> vel;
    std::optional<uint32_t> ch;

    static NoteComparator empty() {
        return NoteComparator{};
    }

    static NoteComparator on(std::optional<NoteNumber> nn
        , std::optional<uint32_t> vel = std::nullopt
        , std::optional<uint32_t> ch = std::nullopt) {
        return NoteComparator{nn, vel, ch};
    }

    static NoteComparator off(std::optional<NoteNumber> nn, std::optional<uint32_t> ch = std::nullopt) {
        return NoteComparator{nn, 0, ch};
    }

    bool equals(const MidiNoteEvent& event) const {
        return compare(nn, vel, ch, event);
    }

    bool equals(const Event& event) const {
        return event.is<MidiNoteEvent>() && equals(event.as<MidiNoteEvent>());
    }

    static bool compare(const std::optional<NoteNumber>& nn
                        , const std::optional<uint32_t>& vel
                        , const std::optional<uint32_t>& ch
                        , const MidiNoteEvent& event) {
        // Note: empty comparison is supported in order to allow checking cases like `matches_sequence({}, {}, {60})`

        if (nn && *nn != event.note_number) return false;

        // if velocity is nullopt, we don't care about its velocity, but we require it to be non-zero
        if ((vel && *vel != event.velocity) || (!vel && event.velocity == 0)) return false;

        if (ch && *ch != event.channel) return false;

        return true;
    }


    static bool compare(const NoteComparator& c, const MidiNoteEvent& event) {
        return compare(c.nn, c.vel, c.ch, event);
    }


    /** compare all notes in a single voice (unordered) */
    static bool matches_chord(const Vec<NoteComparator>& comparators, const Voice<MidiNoteEvent>& voice) {
        auto matched_input = Vec<bool>::zeros(voice.size());
        auto matched_comparators = Vec<bool>::zeros(comparators.size());

        if (voice.size() != comparators.size()) return false;

        for (std::size_t i = 0; i < voice.size(); ++i) {
            for (std::size_t j = 0; j < comparators.size(); ++j) {
                if (comparators[j].equals(voice[i])) {
                    matched_input[j] = true;
                    matched_comparators[i] = true;
                    break;
                }
            }
        }
        return matched_input.as_type<int>().sum() == voice.size()
               && matched_comparators.as_type<int>().sum() == comparators.size();
    }


    static bool matches_chord(const Vec<NoteComparator>& comparators, const Voice<Event>& voice) {
        return matches_chord(comparators, voice_to_midi_events(voice));
    }

    /** compare all notes in a monophonic sequence / multiple monophonic voices (ordered) */
    static bool matches_sequence(const Vec<NoteComparator>& comparators, const Voices<MidiNoteEvent>& voices) {
        if (voices.size() != comparators.size()) return false;

        if (voices.vec().any([&](const Voice<MidiNoteEvent>& voice) {
            return voice.size() != 1;
        })) {
            return false;
        }

        for (std::size_t i = 0; i < voices.size(); ++i) {
            if (voices[i].size() != 1) return false;
            if (!comparators[i].equals(voices[i][0])) return false;

        }

        return true;
    }

    static bool matches_sequence(const Vec<NoteComparator>& comparators, const Voices<Event>& voices) {
        return matches_sequence(comparators, voices_to_midi_events(voices));
    }


    static bool contains(const NoteComparator& comparator, const Voice<MidiNoteEvent>& event) {
        for (const auto& e : event) {
            if (comparator.equals(e)) return true;
        }
        return false;
    }

    static bool contains(const NoteComparator& comparator, const Voice<Event>& event) {
        return contains(comparator, voice_to_midi_events(event));
    }

    static bool contains_chord(const Vec<NoteComparator>& comparators, const Voice<MidiNoteEvent>& event) {
        for (const auto& c : comparators) {
            if (!contains(c, event)) return false;
        }
        return true;
    }

    static bool contains_chord(const Vec<NoteComparator>& comparators, const Voice<Event>& event) {
        return contains_chord(comparators, voice_to_midi_events(event));
    }


    explicit operator std::string() const {
        return "{nn=" + (nn ? std::to_string(*nn) : "any")
               + ", vel=" + (vel ? std::to_string(*vel) : "any")
               + ", ch=" + (ch ? std::to_string(*ch) : "any")
               + "}";
    }

private:
    static MidiNoteEvent to_midi_event(const Event& event) {
        assert(event.is<MidiNoteEvent>());
        return event.as<MidiNoteEvent>();
    }

    static Voice<MidiNoteEvent> voice_to_midi_events(const Voice<Event>& events) {
        return events.as_type<MidiNoteEvent>(to_midi_event);
    }

    static Voices<MidiNoteEvent> voices_to_midi_events(const Voices<Event>& voices) {
        auto v = Vec<Vec<MidiNoteEvent>>::allocated(voices.size());
        for (const auto& voice : voices) {
            v.append(voice_to_midi_events(voice));
        }
        return Voices(v);
    }
};

}

#endif //TESTUTILS_CONDITION_H
