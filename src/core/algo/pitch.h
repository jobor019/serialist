
#ifndef SERIALISTLOOPER_PITCH_H
#define SERIALISTLOOPER_PITCH_H

#include <vector>
#include "vec.h"
#include "core/voice.h"

class Note {
public:
    static const int MIN_NOTE = 21;
    static const int MAX_NOTE = 108;
    static const int NOTE_RANGE = MAX_NOTE - MIN_NOTE;

    enum class Type {
        note_off
        , note_on
    };


    Note(Type type, std::uint8_t pitch) : m_type(type), m_pitch(pitch) {}



    bool operator==(const Note& other) const { return m_type == other.m_type && m_pitch == other.m_pitch; }


    bool is_note_off_for(unsigned int note_on) const {
        return m_type == Note::Type::note_off && m_pitch == note_on;
    }


    Type get_type() const { return m_type; }


    std::uint8_t get_pitch() const { return m_pitch; }


private:

    Type m_type;
    std::uint8_t m_pitch;

};


// ==============================================================================================

class HeldNotes {
public:


    bool bind(std::uint8_t note) {
        if (!m_held.contains(note)) {
            m_held.append(note);
            return true;
        }
        return false;
    }


    bool release(std::uint8_t note) {
        return m_held.remove(note);
    }


    Vec<std::uint8_t> flush() {
        return m_held.drain();
    }


    const Vec<std::uint8_t>& get_held() const {
        return m_held;
    }


private:
    Vec<std::uint8_t> m_held;

};


// ==============================================================================================

class VoicedHeldNotes {
public:
    explicit VoicedHeldNotes(std::size_t num_voices) : m_held_notes(num_voices) {}

    Voices<std::uint8_t> update_num_voices(std::size_t num_voices) {
        if (m_held_notes.size() )
    }


private:
    std::vector<HeldNotes> m_held_notes;

};


#endif //SERIALISTLOOPER_PITCH_H
