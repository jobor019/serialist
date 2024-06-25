

#ifndef GUIUTILS_CLEFS_H
#define GUIUTILS_CLEFS_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "utils.h"

class Clef {
public:
    // TODO: generalize as polymorphic class
    static constexpr int NOTE_BASE_MAP[] = {0, 2, 4, 5, 7, 9, 11};
    static const int NOTE_BASE_MAP_SIZE = 7;
};


class GClef : public Clef {
public:
    static const int NUM_SPACES_8VA = 2;
    static const int NUM_LEDGERS_SPACES_ABOVE = 5;      // 4 ledgers, 5 spaces including a'''
    static const int NUM_STAFF_SPACES = 4;              // 5 lines, 4 spaces
    static const int NUM_LEDGERS_SPACES_BELOW = 4;      // 3 ledgers, 4 spaces including e
    static const int NUM_SPACES_8VB = 2;
    static const int NUM_SPACES = NUM_SPACES_8VA + NUM_LEDGERS_SPACES_ABOVE + NUM_STAFF_SPACES +
                                  NUM_LEDGERS_SPACES_BELOW + NUM_SPACES_8VB;


    // a''' / A6
    static const int TOP_NOTE = 93;

    static const int TOP_NOTE_MAP_POSITION = 5;
    static const int TOP_NOTE_OCTAVE = 7;
    static const int LAST_LEDGER_INDEX_ABOVE = 7;
    static const int FIRST_LEDGER_INDEX_BELOW = 19;
    static const int TOP_INDEX = 0;
    static const int BOTTOM_INDEX = 24;
    static const int REST_POSITION_INDEX = 13;


    explicit GClef(float u = 6.0f)
            : staff_space_px(u)
              , half_staff_space_px(0.5f * u)
              , quarter_staff_space_px(0.25f * u)
              , component_height_px(u * NUM_SPACES)
              , top_padding(u * NUM_SPACES_8VA)
              , bottom_padding(u * NUM_SPACES_8VB)
              , staff_top_y((NUM_SPACES_8VA + NUM_LEDGERS_SPACES_ABOVE) * u)
              , staff_center_y(staff_top_y + 2 * u)
              , staff_bottom_y((NUM_SPACES_8VA + NUM_LEDGERS_SPACES_ABOVE + NUM_STAFF_SPACES) * u)
              , image(juce::ImageFileFormat::loadFrom(juce::File("/Users/joakimborg/Pictures/Notation/gclef.png"))) {}


//    static float compute_component_height_px(float u) {
//        return u * NUM_SPACES;
//    }


    juce::Line<float> get_clef_region() const {
        auto diff = 1.3f * staff_space_px;
        auto start_y = staff_top_y - diff;
        return {0.0f
                , start_y
                , 0.0f
                , start_y + NUM_STAFF_SPACES * staff_space_px + 2 * diff};
    }


    const juce::Image& get_clef_image() const {
        return image;

    }


    juce::Line<float> pitch_index_to_region(int index) const {
        auto start_y = top_padding + static_cast<float>(index) * half_staff_space_px;
        return {0.0f
                , start_y
                , 0.0f
                , start_y + staff_space_px};
    }


    int pitch_y_to_index(float y) const {
        if (y < top_padding) {
            return TOP_INDEX; // TODO: Proper strategy for 8va region

        } else if (y < top_padding + 3.0f * quarter_staff_space_px) {
            // top 0.25 * STAFF_SPACE_PX which normally should be mapped to h''' are mapped to a'''
            return TOP_INDEX;

        } else if (y > component_height_px - bottom_padding) {
            // TODO: Rounding bug for 8vb, may input a d without ledger lines
            return BOTTOM_INDEX;

        } else {
            auto index = static_cast<int>(std::round(2.0f * (y - top_padding) / staff_space_px)) - 1;
            return index;
        }
    }


    juce::Line<float> pitch_y_to_region(float y) const {
        return pitch_index_to_region(pitch_y_to_index(y));
    }


    float pitch_index_to_y(int index) const {
        return staff_space_px * NUM_SPACES_8VA + static_cast<float>(index) * half_staff_space_px;
    }


    float octava_region_height() const {
        return (NUM_SPACES_8VA + NUM_LEDGERS_SPACES_ABOVE) * staff_space_px;
    }


    int num_staff_spaces() const {
        return NUM_STAFF_SPACES;
    }

    int total_num_spaces() const {
        return NUM_SPACES;
    }


    float rest_center_y() const {
        return staff_center_y;
    }


    std::vector<float> ledger_lines_y(int min_pitch_index, int max_pitch_index) const {
        int num_lines_above = std::max(0, NUM_LEDGERS_SPACES_ABOVE - 1 - min_pitch_index / 2);
        int num_lines_below = std::min(NUM_LEDGERS_SPACES_BELOW - 1
                                       , (2 + max_pitch_index - FIRST_LEDGER_INDEX_BELOW) / 2);

        std::vector<float> ys;

        for (int i = 0; i < num_lines_above; ++i) {
            ys.emplace_back(pitch_index_to_y(LAST_LEDGER_INDEX_ABOVE - 2 * i) + half_staff_space_px);
        }

        for (int i = 0; i < num_lines_below; ++i) {
            ys.emplace_back(pitch_index_to_y(FIRST_LEDGER_INDEX_BELOW + 2 * i) + half_staff_space_px);
        }

        return ys;
    }


    int pitch_y_to_base(float y) const {
        auto index = pitch_y_to_index(y);

        int pitch_class = utils::modulo(TOP_NOTE_MAP_POSITION - index, Clef::NOTE_BASE_MAP_SIZE);
        auto relative_octave = (Clef::NOTE_BASE_MAP_SIZE - TOP_NOTE_MAP_POSITION - 1 + index) / NOTE_BASE_MAP_SIZE;
        auto base = TOP_NOTE + (Clef::NOTE_BASE_MAP[pitch_class] - Clef::NOTE_BASE_MAP[TOP_NOTE_MAP_POSITION]) -
                    12 * relative_octave;

        return base;
    }


    int pitch_base_to_index(int pitch_base) {
        auto alignment_point = TOP_NOTE - Clef::NOTE_BASE_MAP[TOP_NOTE_MAP_POSITION];
        auto pitch_diff = pitch_base - alignment_point;
        auto relative_octave = std::abs(static_cast<int>(std::floor(static_cast<float>(pitch_diff) / 12.0f)));
        auto pitch_class = utils::modulo(pitch_diff, 12);
        auto relative_pitch_index = static_cast<int>(
                std::distance(Clef::NOTE_BASE_MAP
                              , std::find(Clef::NOTE_BASE_MAP
                                          , Clef::NOTE_BASE_MAP + Clef::NOTE_BASE_MAP_SIZE
                                          , pitch_class))
        );
        auto index = TOP_NOTE_MAP_POSITION - relative_pitch_index + relative_octave * NOTE_BASE_MAP_SIZE;
//        std::cout << "index: " << index << "(rel_8v: " << relative_octave << ", rel_pc: " << relative_pitch_index
//                  << ")\n";
        return index;

    }


    int component_height() const {
        return static_cast<int>(component_height_px);
    }


private:
    float staff_space_px;
    float half_staff_space_px;
    float quarter_staff_space_px;
    float component_height_px;
    float top_padding;
    float bottom_padding;
    float staff_top_y;
    float staff_center_y;
    float staff_bottom_y;

    juce::Image image;
};
#endif //GUIUTILS_CLEFS_H
