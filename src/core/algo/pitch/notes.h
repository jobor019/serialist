
#ifndef SERIALISTLOOPER_NOTES_H
#define SERIALISTLOOPER_NOTES_H

#include <vector>
#include "core/algo/collections/vec.h"
#include "core/algo/collections/voices.h"
#include "core/algo/collections/held.h"
#include "core/algo/random.h"


namespace pitch {
static const int MIN_NOTE = 21;
static const int MAX_NOTE = 108;
static const int NOTE_RANGE = MAX_NOTE - MIN_NOTE;
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


    bool is_in(NoteNumber note) const {
        return m_mask[utils::modulo(note - m_transposition, m_pivot)];
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


    NoteNumber select_from(NoteNumber start
                           , NoteNumber end
                           , const PitchClassRange& enabled_pitch_classes) {
        return m_random.choice(Vec<NoteNumber>::range(start, end)
                                       .filter([&enabled_pitch_classes](NoteNumber note) {
                                           return enabled_pitch_classes.is_in(note);
                                       }));
    }


private:
    Random m_random;

};


class Note {
public:


    enum class Type {
        note_off
        , note_on
    };


    Note(Type type, NoteNumber pitch) : m_type(type), m_pitch(pitch) {}


    static Note note_on(NoteNumber pitch) { return {Type::note_on, pitch}; }


    static Note note_off(NoteNumber pitch) { return {Type::note_off, pitch}; }


    bool operator==(const Note& other) const { return m_type == other.m_type && m_pitch == other.m_pitch; }


    bool is_note_off_for(NoteNumber note_on) const {
        return m_type == Note::Type::note_off && m_pitch == note_on;
    }


    Type get_type() const { return m_type; }


    NoteNumber get_pitch() const { return m_pitch; }


private:

    Type m_type;
    NoteNumber m_pitch;

};


// ==============================================================================================

using HeldNotes = Held<NoteNumber>;

using MultiVoiceHeldNotes = MultiVoiceHeld<NoteNumber>;


#endif //SERIALISTLOOPER_NOTES_H
