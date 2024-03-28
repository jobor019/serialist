
#ifndef SERIALISTLOOPER_ALLOCATOR_H
#define SERIALISTLOOPER_ALLOCATOR_H


#include <map>
#include "core/algo/weighted_random.h"
#include "core/collections/vec.h"
#include "core/collections/held.h"
#include "core/algo/pitch/notes.h"
#include "core/algo/classifiers.h"
#include "core/algo/histogram.h"
#include "core/algo/random.h"
#include "core/algo/partial_note.h"
#include "core/generatives/stereotypes/base_stereotypes.h"
#include "core/algo/time/trigger.h"

class Allocator {
public:
    explicit Allocator(std::optional<unsigned int> seed = std::nullopt) : m_random(seed), m_pitch_selector(seed) {}


    Voices<NoteNumber> release(const Voices<Trigger>& triggers, std::size_t num_voices) {
        if (m_configuration_changed)
            update_configuration();

        auto note_offs = Voices<NoteNumber>::zeros(num_voices);
        for (std::size_t i = 0; i < num_voices; ++i) {
            if (triggers[i].contains_any({Trigger::pulse_on, Trigger::pulse_off})) {
                note_offs[i].extend(m_currently_held.flush(i));
            }
        }
        return note_offs;
    }


    Voices<NoteNumber> bind(const Voices<Trigger>& triggers, std::size_t num_voices) {
        if (m_configuration_changed)
            update_configuration();

        auto classes = m_classifier.classify(m_currently_held.get_held().flattened());
        auto histogram = Histogram<std::size_t>::with_discrete_bins(
                classes
                , Vec<std::size_t>::range(m_classifier.get_num_classes())
        );
        auto counts = histogram.get_counts().cloned();
        auto weights = m_spectrum_distribution.cloned()
                .multiply(0.0, m_invalid_bins)
                .normalize_l1()
                .multiply(static_cast<double>(num_voices));

        auto output = Voices<NoteNumber>::zeros(num_voices);

        for (std::size_t voice = 0; voice < num_voices; ++voice) {
            if (triggers[voice].contains(Trigger::pulse_on)) {
                auto distribution = (weights - counts.as_type<double>()).clip({0}, std::nullopt);
                auto class_idx = m_random.weighted_choice(distribution);
                counts[class_idx] += 1;
                auto note = m_pitch_selector.select_from(m_classifier.start_of(class_idx)
                                                         , m_classifier.end_of(class_idx)
                                                         , m_enabled_pitch_classes);
                if (note.has_value()) {
                    m_currently_held.bind(*note, voice);
                    output[voice].append(*note);
                }
            }
        }

        return output;
    }


    Voices<NoteNumber> flush(bool non_matching_only = false) {
        if (non_matching_only) {
            return m_currently_held.flush([this](const auto& note) { return !m_enabled_pitch_classes.is_in(note); });
        } else {
            return m_currently_held.flush();
        }
    }


    Voices<NoteNumber> resize(std::size_t num_voices) {
        return m_currently_held.resize(num_voices);
    }


    void set_pitch_classes(const PitchClassRange& pcs) {
        m_enabled_pitch_classes = pcs;
        m_configuration_changed = true;
    }


    void set_spectrum_distribution(const Vec<double>& spectrum_distribution) {
        m_spectrum_distribution = spectrum_distribution;
        m_classifier.set_num_classes(spectrum_distribution.size());
        m_configuration_changed = true;
    }


    const LinearBandClassifier<NoteNumber>& get_classifier() const {
        return m_classifier;
    }


    const Vec<double>& get_spectrum_distribution() const {
        return m_spectrum_distribution;
    }


    const PitchSelector& get_pitch_selector() const {
        return m_pitch_selector;
    }


private:
    void update_configuration() {
        auto num_classes = m_classifier.get_num_classes();

        Vec<std::size_t> enabled_bands;
        for (std::size_t cls = 0; cls < num_classes; ++cls) {
            auto [lo, hi] = m_classifier.bounds_of(cls);
            for (NoteNumber pc = lo; pc < hi; ++pc) {
                if (m_enabled_pitch_classes.is_in(pc)) {
                    enabled_bands.append(cls);
                    break;
                }
            }
        }
        m_invalid_bins = enabled_bands.boolean_mask(num_classes).logical_not();
        m_configuration_changed = false;
    }


    PitchClassRange m_enabled_pitch_classes;

    MultiVoiceHeld<NoteNumber> m_currently_held{1};
    LinearBandClassifier<NoteNumber> m_classifier{pitch::MIN_NOTE, pitch::MAX_NOTE, 12};

    Vec<double> m_spectrum_distribution = Vec<double>::repeated(m_classifier.get_num_classes(), 1.0);
    Vec<bool> m_invalid_bins = Vec<bool>::repeated(m_classifier.get_num_classes(), false);

    Random m_random;
    PitchSelector m_pitch_selector;


    bool m_configuration_changed = false;
};


// ==============================================================================================

class AllocatorNode : public NodeBase<PartialNote> {
public:

    class AllocatorKeys {
    public:
        static const inline std::string PULSE = "pulse_on";
        static const inline std::string PITCH_CLASSES = "pitch_classes";
        static const inline std::string PIVOT = "pivot";
        static const inline std::string DISTRIBUTION = "distribution";
        static const inline std::string FLUSH_ON_CHANGE = "flush_on_change";

        static const inline std::string CLASS_NAME = "distributor";
    };


    AllocatorNode(const std::string& id
                  , ParameterHandler& parent
                  , Node<Trigger>* pulse
                  , Node<Facet>* pitch_classes
                  , Node<Facet>* pivot
                  , Node<Facet>* distribution
                  , Node<Facet>* flush_on_change
                  , Node<Facet>* enabled
                  , Node<Facet>* num_voices)
            : NodeBase<PartialNote>(id, parent, enabled, num_voices, AllocatorKeys::CLASS_NAME)
              , m_pulse(add_socket(AllocatorKeys::PULSE, pulse))
              , m_pitch_classes(add_socket(AllocatorKeys::PITCH_CLASSES, pitch_classes))
              , m_pivot(add_socket(AllocatorKeys::PIVOT, pivot))
              , m_distribution(add_socket(AllocatorKeys::DISTRIBUTION, distribution))
              , m_flush_on_change(add_socket(AllocatorKeys::FLUSH_ON_CHANGE, flush_on_change)) {}


    Voices<PartialNote> process() override {
        if (auto t = pop_time(); !t) // process has already been called this cycle
            return m_current_value;

        if (!(is_enabled() && m_pulse.is_connected()
              && m_pitch_classes.is_connected() && m_distribution.is_connected())) {
            m_current_value = Voices<PartialNote>::empty_like();

            // first callback since it was disabled: flush any held notes
            if (m_previous_enable_state) {
                m_previous_enable_state = false;
                m_current_value = as_partial(m_allocator.flush(), Trigger::pulse_off);
            }

            return m_current_value;
        }

        auto triggers = m_pulse.process();

        if (triggers.is_empty_like()) {
            return m_current_value;
        }

        auto num_voices = voice_count(triggers.size());

        auto output = Voices<PartialNote>::zeros(num_voices);

        if (num_voices != m_previous_num_voices) {
            output.merge_uneven(as_partial(m_allocator.resize(num_voices), Trigger::pulse_off), true);
        }

        triggers.adapted_to(num_voices);

        if (auto flushed = handle_pitch_classes()) {
            output.merge_uneven(*flushed, false);
        }
        handle_distribution();

        output.merge_uneven(as_partial(m_allocator.release(triggers, num_voices), Trigger::pulse_off), false);
        output.merge_uneven(as_partial(m_allocator.bind(triggers, num_voices), Trigger::pulse_on), false);

        m_current_value = output;
        return m_current_value;
    }


private:
    static Voices<PartialNote> as_partial(const Voices<NoteNumber>& notes, Trigger trigger_type) {
        return notes.as_type<PartialNote>([&trigger_type](const auto& note) {
            return PartialNote(trigger_type, Facet(note));
        });
    }


    std::optional<Voices<PartialNote>> handle_pitch_classes() {
        if (m_pitch_classes.has_changed() || m_pivot.has_changed()) {
            auto pivot = static_cast<NoteNumber>(m_pivot.process().first_or(12));
            auto pcs = m_pitch_classes.process()
                    .firsts()
                    .filter([&pivot](const auto& pc) { return pc.has_value() && *pc < pivot; })
                    .as_type<NoteNumber>([](const auto& pc) { return static_cast<NoteNumber>(*pc); });

            if (pcs.empty()) {
                pcs = Vec<NoteNumber>::range(pivot);
            }

            m_allocator.set_pitch_classes(PitchClassRange(pcs, pivot));


            if (auto flush_on_change = m_flush_on_change.process().first_or(false); flush_on_change) {
                return {as_partial(m_allocator.flush(true), Trigger::pulse_off)};
            }
        }
        return std::nullopt;
    }


    void handle_distribution() {
        if (m_distribution.has_changed()) {
            auto distribution = m_distribution.process().firsts_or(0.0);
            m_allocator.set_spectrum_distribution(distribution);
        }

    }


    Allocator m_allocator;

    Socket<Trigger>& m_pulse;
    Socket<Facet>& m_pitch_classes;
    Socket<Facet>& m_pivot;
    Socket<Facet>& m_distribution;
    Socket<Facet>& m_flush_on_change;

    Voices<PartialNote> m_current_value = Voices<PartialNote>::empty_like();

    bool m_previous_enable_state = true;
    std::size_t m_previous_num_voices = 0;

};

#endif //SERIALISTLOOPER_ALLOCATOR_H
