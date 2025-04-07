
#ifndef SERIALISTLOOPER_NOTES_H
#define SERIALISTLOOPER_NOTES_H

#include <vector>
#include "core/collections/vec.h"
#include "core/collections/voices.h"
#include "core/collections/held.h"
#include "core/algo/random/random.h"
#include "core/utility/math.h"

namespace serialist {

namespace pitch {
static const unsigned int MIN_NOTE = 21;
static const unsigned int MAX_NOTE = 108;
static const unsigned int NOTE_RANGE = MAX_NOTE - MIN_NOTE;
} // namespace pitch


// ==============================================================================================

using NoteNumber = unsigned int;
using MidiCents = unsigned int;


// ==============================================================================================

class PitchClassRange {
public:
    explicit PitchClassRange(const Vec<NoteNumber>& enabled_pitch_classes = Vec<NoteNumber>::range(12)
                             , NoteNumber pivot = 12
                             , NoteNumber transposition = 0)
            : m_classes(enabled_pitch_classes)
              , m_pivot(pivot)
              , m_transposition(transposition)
              , m_mask(m_classes.boolean_mask(m_pivot)) {}


    static PitchClassRange with(const PitchClassRange& other, const Vec<NoteNumber>& new_pitch_classes) {
        return PitchClassRange{new_pitch_classes, other.m_pivot, other.m_transposition};
    }


    static PitchClassRange with(const PitchClassRange& other, NoteNumber new_pivot) {
        return PitchClassRange{other.m_classes, new_pivot, other.m_transposition};
    }


    bool is_in(NoteNumber note) const {
        return m_mask[classify(note)];
    }


    NoteNumber classify(NoteNumber note) const {
        return utils::modulo(note - m_transposition, m_pivot);
    }


    const Vec<NoteNumber>& get_classes() const {
        return m_classes;
    }


    NoteNumber get_pivot() const {
        return m_pivot;
    }


private:
    Vec<NoteNumber> m_classes;
    NoteNumber m_pivot;
    NoteNumber m_transposition;

    Vec<bool> m_mask;
};


// ==============================================================================================

class PitchSelector {
public:

    explicit PitchSelector(std::optional<unsigned int> seed = std::nullopt) : m_random(seed) {}


    std::optional<NoteNumber> select_from(NoteNumber start
                                          , NoteNumber end
                                          , const PitchClassRange& enabled_pitch_classes) {
        auto pitches = Vec<NoteNumber>::range(start, end)
                .filter([&enabled_pitch_classes](NoteNumber note) {
                    return enabled_pitch_classes.is_in(note);
                });

        if (pitches.empty())
            return std::nullopt;

        return {m_random.choice(pitches)};
    }


private:
    Random m_random;

};


//class Note {
//public:
//    enum class Type {
//        note_off
//        , note_on
//    };
//
//
//    Note(Type type, NoteNumber pitch) : m_type(type), m_pitch(pitch) {}
//
//
//    static Note note_on(NoteNumber pitch) { return {Type::note_on, pitch}; }
//
//
//    static Note note_off(NoteNumber pitch) { return {Type::note_off, pitch}; }
//
//
//    bool operator==(const Note& other) const { return m_type == other.m_type && m_pitch == other.m_pitch; }
//
//
//    bool is_note_off_for(NoteNumber note_on) const {
//        return m_type == Note::Type::note_off && m_pitch == note_on;
//    }
//
//
//    Type get_type() const { return m_type; }
//
//
//    NoteNumber get_pitch() const { return m_pitch; }
//
//
//private:
//    Type m_type;
//    NoteNumber m_pitch;
//};


// ==============================================================================================

struct ChanneledHeld {
    NoteNumber note;
    unsigned int channel;
};

struct IdentifiedChanneledHeld {
    std::size_t id;
    NoteNumber note;
    unsigned int channel;

    bool operator==(const IdentifiedChanneledHeld& other) const {
        return id == other.id;
    }
};

using HeldNotes = Held<ChanneledHeld>;
using HeldNotesWithIds = Held<IdentifiedChanneledHeld, true>;

using MultiVoiceHeldNotes = MultiVoiceHeld<ChanneledHeld>;

} // namespace serialist

#endif //SERIALISTLOOPER_NOTES_H
