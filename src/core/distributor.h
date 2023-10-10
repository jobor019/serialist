
#ifndef SERIALISTLOOPER_DISTRIBUTOR_H
#define SERIALISTLOOPER_DISTRIBUTOR_H


#include <map>
//#include "node_base.h"
#include "core/algo/weighted_random.h"
#include "core/algo/collections/vec.h"
#include "core/algo/collections/held.h"
#include "core/algo/pitch/notes.h"
#include "core/algo/classifiers.h"
#include "core/algo/stat.h"
#include "core/algo/random.h"
#include "events.h"

class Allocator {
public:
    explicit Allocator(std::optional<unsigned int> seed = std::nullopt) : m_random(seed), m_pitch_selector(seed) {}


    Voices<NoteNumber> release(const Voices<Trigger>& triggers, std::size_t num_voices) {
        if (m_configuration_changed)
            update_configuration();

        auto note_offs = Voices<NoteNumber>::zeros(num_voices);
        for (std::size_t i = 0; i < num_voices; ++i) {
            if (triggers[i].contains([](const Trigger& t) {
                return t.get_type() == Trigger::Type::pulse_off || t.get_type() == Trigger::Type::pulse_on;
            })) {
                note_offs[i].extend(m_currently_held.flush(i));
            }
        }
        return note_offs;
    }


    Voices<NoteNumber> bind(const Voices<Trigger>& triggers, std::size_t num_voices) {
        if (m_configuration_changed)
            update_configuration();

        auto classes = m_classifier.classify(m_currently_held.get_held().flattened());
        auto histogram = Histogram<std::size_t>(classes, Vec<std::size_t>::range(m_classifier.get_num_classes()));
        auto counts = histogram.get_counts().cloned().multiply(0UL, m_invalid_bins);

        auto output = Voices<NoteNumber>::zeros(num_voices);

        for (std::size_t voice = 0; voice < num_voices; ++voice) {
            if (triggers[voice].contains([](const Trigger& t) { return t.get_type() == Trigger::Type::pulse_on; })) {
                auto weights = m_spectrum_distribution - counts.as_type<double>().normalize();
                auto class_idx = m_random.weighted_choice(weights);
                counts[class_idx] -= 1;
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


    Voices<NoteNumber> flush() {
        return m_currently_held.flush();
    }


    Voices<NoteNumber> resize(std::size_t num_voices) {
        return m_currently_held.resize(num_voices);
    }


    void set_pitch_classes(const PitchClassRange& pcs) {
        m_enabled_pitch_classes = pcs;
        m_configuration_changed = true;
    }


    void set_spectrum_distribution(const Vec<double>& spectrum_distribution) {
        m_spectrum_distribution = spectrum_distribution.cloned().normalize();
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
        m_spectrum_distribution.normalize();
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
        m_invalid_bins = enabled_bands.boolean_mask(num_classes);
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

//
//// ==============================================================================================
//
//class DistributorNode : public NodeBase<PartialNote> {
//public:
//
//    class DistributorKeys {
//    public:
//        static const inline std::string PULSE = "pulse_on";
//        static const inline std::string MATERIAL = "material";
//        static const inline std::string PIVOT = "pivot";
//        static const inline std::string DISTRIBUTION = "distribution";
//        static const inline std::string FLUSH_ON_CHANGE = "flush_on_change";
//
//        static const inline std::string CLASS_NAME = "distributor";
//    };
//
//
//    DistributorNode(const std::string& id
//                    , ParameterHandler& parent
//                    , Node<Trigger>* pulse
//                    , Node<Facet>* material
//                    , Node<Facet>* pivot
//                    , Node<Facet>* distribution
//                    , Node<Facet>* flush_on_change
//                    , Node<Facet>* enabled
//                    , Node<Facet>* num_voices)
//            : NodeBase<PartialNote>(id, parent, enabled, num_voices, DistributorKeys::CLASS_NAME)
//              , m_pulse(add_socket(DistributorKeys::PULSE, pulse))
//              , m_material(add_socket(DistributorKeys::MATERIAL, material))
//              , m_pivot(add_socket(DistributorKeys::PIVOT, pivot))
//              , m_distribution(add_socket(DistributorKeys::DISTRIBUTION, distribution))
//              , m_flush_on_change(add_socket(DistributorKeys::FLUSH_ON_CHANGE, flush_on_change)) {}
//
//
//    Voices<PartialNote> process() override {
//        throw std::runtime_error("not implemented: "); // TODO: implement
//    }
//
//
//private:
//    Socket<Trigger>& m_pulse;
//    Socket<Facet>& m_material;
//    Socket<Facet>& m_pivot;
//    Socket<Facet>& m_distribution;
//    Socket<Facet>& m_flush_on_change;
//
//};

#endif //SERIALISTLOOPER_DISTRIBUTOR_H
