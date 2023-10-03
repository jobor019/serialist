
#ifndef SERIALISTLOOPER_DISTRIBUTOR_H
#define SERIALISTLOOPER_DISTRIBUTOR_H


#include <map>
#include "partial_note.h"
#include "node_base.h"
#include "core/algo/weighted_random.h"
#include "core/algo/held_notes.h"
#include "core/algo/vec.h"

class Distributor {
public:
    Voices<int> process(const Voices<Trigger>& triggers, std::size_t num_voices) {
    }

    Voi

private:
    Vec<int> m_pitch_material;
    int m_pitch_pivot;

    Vec<double> m_spectrum_distribution;

    std::size_t m_num_voices;


    Held<int> m_currently_held;

    bool m_configuration_changed = false;
};


class DistributorOld {
public:
    static const int MIN_NOTE = 21;
    static const int MAX_NOTE = 108;
    static const int NOTE_RANGE = MAX_NOTE - MIN_NOTE;

//    struct NoteVector {
//
//        explicit NoteVector(std::size_t num_voices) : notes(num_voices, std::nullopt) {}
//
//
//        explicit NoteVector(const std::vector<std::optional<int>>& notes) : notes(notes) {}
//
//
//        std::size_t size() const { return notes.size(); }
//
//
//        NoteVector& operator+=(const NoteVector& other) {
//            assert(size() == other.size());
//
//            for (std::size_t i = 0; i < other.size(); ++i) {
//                if (other.notes.at(i)) {
//                    notes.at(i) = other.notes.at(i);
//                }
//            }
//            notes.insert(notes.end(), other.notes.begin(), other.notes.end());
//            return *this;
//        }
//
//
//        std::vector<std::optional<int>> notes;
//    };
//
//    struct Notes {
//        Notes(const NoteVector& note_offs
//              , const NoteVector& note_ons)
//                : note_offs(note_offs), note_ons(note_ons) {
//            assert(note_offs.size() == note_ons.size());
//        }
//
//
//        NoteVector note_offs;
//        NoteVector note_ons;
//    };


    Notes process(const Voices<Trigger>& triggers
                  , const std::vector<int>& material
                  , int pivot
                  , const std::vector<double>& distribution
                  , bool flush_on_change
                  , bool enabled
                  , std::size_t num_voices) {

        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
        // TODO
        // TODO: better solution than passing all arguments each time would be to update its internal state
        // TODO:    only if the value changes (using Socket.process_if_changed()).
        // TODO
        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO
        // TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO

        auto note_offs = trigger_note_offs(triggers, material, flush_on_change, enabled, num_voices);
        auto note_ons = trigger_note_ons(triggers, material, pivot, distribution, enabled, num_voices);

        m_previous_enabled_state = enabled;
        m_previous_material = material;
        m_previous_num_voices = num_voices;

        return {note_offs, note_ons};
    }


private:


    NoteVector trigger_note_offs(const Voices<Trigger>& triggers
                                 , const std::vector<int>& material
                                 , bool flush_on_change
                                 , bool enabled
                                 , std::size_t num_voices) {
        if (!enabled && enabled != m_previous_enabled_state) {
            return flush_all();
        }


        NoteVector note_offs(num_voices);
        if (material_changed(material) && flush_on_change) {
            note_offs += flush_invalid(material);
        }

        if (num_voices < m_previous_num_voices) {
            note_offs += flush_old_voices(num_voices);
        }

        note_offs += generate_note_offs(triggers);

        return note_offs;
    }


    NoteVector trigger_note_ons(const Voices<Trigger>& triggers
                                , const std::vector<int>& material
                                , int pivot
                                , const std::vector<double>& distribution
                                , bool enabled
                                , std::size_t num_voices) {
        if (!enabled) {
            return NoteVector(num_voices);
        }

        auto held = get_held_distribution(distribution.size());
        auto pdf = get_diff_distribution(held, distribution);
        pdf = disable_invalid_bins(distribution, material);

        NoteVector note_ons(num_voices);

        for (const auto& voice_index: retriggers(triggers)) {
            auto cdf = DiscreteWeightedRandom(pdf);
            auto bin = cdf.next();
            pdf.at(bin) -= 1.0;
            note_ons.notes.at(voice_index) = select_from(bin, material, pivot, distribution.size());
        }

        return note_ons;
    }


    std::vector<double> get_held_distribution(std::size_t num_bins) {
        throw std::runtime_error("not implemented: "); // TODO

    }


    std::vector<double> get_diff_distribution(const std::vector<double>& held_distribution
                                              , const std::vector<double>& target_distribution) {
        throw std::runtime_error("not implemented: "); // TODO
    }


    std::vector<double> disable_invalid_bins(const std::vector<double>& distribution
                                             , const std::vector<int>& material) {
        throw std::runtime_error("not implemented: "); // TODO
    }


    std::vector<std::size_t> retriggers(const Voices<Trigger>& triggers) {
        throw std::runtime_error("not implemented: "); // TODO
    }


    bool material_changed(const std::vector<int>& material) {
        return material == m_previous_material;
    }


    NoteVector flush_all() {
        throw std::runtime_error("not implemented: "); // TODO

    }


    NoteVector flush_invalid(const std::vector<int>& new_material) {
        throw std::runtime_error("not implemented: "); // TODO
    }


    NoteVector flush_old_voices(std::size_t new_num_voices) {
        throw std::runtime_error("not implemented: "); // TODO
    }


    NoteVector generate_note_offs(const Voices<Trigger>& triggers) {
        throw std::runtime_error("not implemented: "); // TODO
    }


    std::size_t bin_of(int note_number, std::size_t num_bins) {
//        return
    }


    int select_from(std::size_t bin, const std::vector<int>& material, int pivot, std::size_t num_bins) {
        auto [low, high] = range_of(bin, num_bins);

    }


    static std::pair<int, int> range_of(std::size_t bin, std::size_t num_bins) {
        double start = static_cast<double>(bin) / static_cast<double>(num_bins);
        double end = static_cast<double>(bin + 1) / static_cast<double>(num_bins);
        return {MIN_NOTE + static_cast<int>(std::round(start * NOTE_RANGE))
                , MIN_NOTE + static_cast<int>(std::round(end * NOTE_RANGE))};
    }

    std::map<std::size_t, std::vector<int>> full_material(const std::vector<int>& material, std::size_t num_bins) {
        throw std::runtime_error("not implemented: "); // TODO

    }


    bool m_previous_enabled_state = false;
    Held<int> m_currently_held;

    std::vector<int> m_previous_material;
    std::size_t m_previous_num_voices;
};

class DistributorNode : public NodeBase<PartialNote> {
public:

    class DistributorKeys {
    public:
        static const inline std::string PULSE = "pulse";
        static const inline std::string MATERIAL = "material";
        static const inline std::string PIVOT = "pivot";
        static const inline std::string DISTRIBUTION = "distribution";
        static const inline std::string FLUSH_ON_CHANGE = "flush_on_change";

        static const inline std::string CLASS_NAME = "distributor";
    };


    DistributorNode(const std::string& id
                    , ParameterHandler& parent
                    , Node<Trigger>* pulse
                    , Node<Facet>* material
                    , Node<Facet>* pivot
                    , Node<Facet>* distribution
                    , Node<Facet>* flush_on_change
                    , Node<Facet>* enabled
                    , Node<Facet>* num_voices)
            : NodeBase<PartialNote>(id, parent, enabled, num_voices, DistributorKeys::CLASS_NAME)
              , m_pulse(add_socket(DistributorKeys::PULSE, pulse))
              , m_material(add_socket(DistributorKeys::MATERIAL, material))
              , m_pivot(add_socket(DistributorKeys::PIVOT, pivot))
              , m_distribution(add_socket(DistributorKeys::DISTRIBUTION, distribution))
              , m_flush_on_change(add_socket(DistributorKeys::FLUSH_ON_CHANGE, flush_on_change)) {}


    Voices<PartialNote> process() override {
        throw std::runtime_error("not implemented: "); // TODO: implement
    }


private:
    Socket<Trigger>& m_pulse;
    Socket<Facet>& m_material;
    Socket<Facet>& m_pivot;
    Socket<Facet>& m_distribution;
    Socket<Facet>& m_flush_on_change;

};

#endif //SERIALISTLOOPER_DISTRIBUTOR_H
