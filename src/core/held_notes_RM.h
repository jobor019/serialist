//
//#ifndef SERIALISTLOOPER_HELD_NOTES_H
//#define SERIALISTLOOPER_HELD_NOTES_H
//
//class HeldNotes {
//public:
//
//    void append(const MidiEvent& note_on_or_off) {
//        if (note_on_or_off.is_note_on()) {
//            if (!is_held(note_on_or_off)) {
//                m_held_notes.push_back(note_on_or_off);
//            }
//        } else {
//            remove_if(note_on_or_off);
//        }
//    }
//
//    void extend(const std::vector<MidiEvent>& note_ons_or_offs) {
//        for (auto& event : note_ons_or_offs) {
//            append(event);
//        }
//    }
//
//
//    std::vector<MidiEvent> flush(const TimePoint& t) {
//        std::vector<MidiEvent> outlet_note_offs;
//        outlet_note_offs.reserve(m_held_notes.size());
//
//        for (auto& note_on : m_held_notes) {
//            outlet_note_offs.emplace_back(MidiEvent::note_off(t.get_tick(), note_on.get_midi_cents(), note_on.get_channel()));
//        }
//
//        m_held_notes.clear();
//
//        return outlet_note_offs;
//    }
//
//
//    bool is_held(const MidiEvent& note_on) {
//        return is_held(note_on.get_midi_cents(), note_on.get_channel());
//    }
//
//
//    bool is_held(int midi_cents, int channel) {
//        return std::any_of(m_held_notes.begin(), m_held_notes.end()
//                           , [&](auto& note) { return note.matches(midi_cents, channel); });
//    }
//
//    const std::vector<MidiEvent>& held() const {
//        return m_held_notes;
//    }
//
//
//private:
//    void remove_if(const MidiEvent& note_off) {
//        m_held_notes.erase(
//                std::remove_if(
//                        m_held_notes.begin()
//                        , m_held_notes.end()
//                        , [&note_off](const auto& note) { return note.matches(note_off); }
//                ), m_held_notes.end());
//    }
//
//
//    std::vector<MidiEvent> m_held_notes;
//
//
//};
//
//#endif //SERIALISTLOOPER_HELD_NOTES_H
