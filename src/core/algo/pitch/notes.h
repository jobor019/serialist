
#ifndef SERIALISTLOOPER_NOTES_H
#define SERIALISTLOOPER_NOTES_H

#include <vector>
#include "core/algo/collections/vec.h"
#include "core/algo/collections/voices.h"
#include "core/algo/collections/held.h"

class Note {
public:
    static const int MIN_NOTE = 21;
    static const int MAX_NOTE = 108;
    static const int NOTE_RANGE = MAX_NOTE - MIN_NOTE;

    enum class Type {
        note_off
        , note_on
    };


    Note(Type type, unsigned int pitch) : m_type(type), m_pitch(pitch) {}


    static Note note_off(unsigned int pitch) { return Note(Type::note_off, pitch); }


    bool operator==(const Note& other) const { return m_type == other.m_type && m_pitch == other.m_pitch; }


    bool is_note_off_for(unsigned int note_on) const {
        return m_type == Note::Type::note_off && m_pitch == note_on;
    }


    Type get_type() const { return m_type; }


    unsigned int get_pitch() const { return m_pitch; }


private:

    Type m_type;
    unsigned int m_pitch;

};


// ==============================================================================================

using HeldNotes = Held<unsigned int>;

using MultiVoiceHeldNotes = MultiVoiceHeld<unsigned int>;

//
//// ==============================================================================================
//
//class VoicedHeldNotes {
//public:
//    explicit VoicedHeldNotes(std::size_t num_voices) : m_held_notes(num_voices) {}
//
//    Voices<std::uint8_t> update_num_voices(std::size_t num_voices) {
//        if (m_held_notes.size() )
//    }
//
//
//private:
//    Vec<HeldNotes> m_held_notes;
//
//};


#endif //SERIALISTLOOPER_NOTES_H
