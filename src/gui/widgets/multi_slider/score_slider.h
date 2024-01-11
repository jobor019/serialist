//
//#ifndef SERIALISTLOOPER_SCORE_SLIDER_H
//#define SERIALISTLOOPER_SCORE_SLIDER_H
//
//#include "multi_slider_base.h"
//
//class ScoreDimensions {
//public:
//    static constexpr float CLEF_PADDING_BEFORE_SPACES = 1.0f;
//    static constexpr float CLEF_WIDTH_SPACES = 3.0f;
//    static constexpr float CLEF_PADDING_AFTER_SPACES = 1.0f;
//    static constexpr float CLEF_TOTAL_WIDTH_SPACES = CLEF_PADDING_BEFORE_SPACES +
//                                                     CLEF_WIDTH_SPACES +
//                                                     CLEF_PADDING_AFTER_SPACES;
//
//    static constexpr float STEP_ACCIDENTAL_PADDING_SPACES = 0.0f;
//    static constexpr float STEP_ACCIDENTAL_WIDTH_SPACES = 2.0f;
//    static constexpr float STEP_NOTEHEAD_PADDING_BEFORE_SPACES = 0.0f;
//    static constexpr float STEP_NOTEHEAD_WIDTH_SPACES = 2.0f;
//    static constexpr float STEP_NOTEHEAD_PADDING_AFTER_SPACES = 0.0f;
//    static constexpr float STEP_TOTAL_WIDTH_SPACES = STEP_ACCIDENTAL_PADDING_SPACES +
//                                                     STEP_ACCIDENTAL_WIDTH_SPACES +
//                                                     STEP_NOTEHEAD_PADDING_BEFORE_SPACES +
//                                                     STEP_NOTEHEAD_WIDTH_SPACES +
//                                                     STEP_NOTEHEAD_PADDING_AFTER_SPACES;
//
//
//    explicit ScoreDimensions(float u = 6.0f)
//            : staff_space_px(u)
//              , half_staff_space_px(0.5f * u)
//              , quarter_staff_space_px(0.25f * u)
//              , staff_line_thickness_px(u / 7.5f)
//
//              , clef_padding_before_px(CLEF_PADDING_BEFORE_SPACES * u)
//              , clef_padding_after_px(CLEF_PADDING_AFTER_SPACES * u)
//              , clef_width_px(CLEF_WIDTH_SPACES * u)
//              , clef_total_width_px(clef_padding_before_px + clef_padding_after_px + clef_width_px)
//
//              , step_accidental_padding_before_px(STEP_ACCIDENTAL_PADDING_SPACES * u)
//              , step_accidental_width_px(STEP_ACCIDENTAL_WIDTH_SPACES * u)
//              , step_notehead_padding_before_px(STEP_NOTEHEAD_PADDING_BEFORE_SPACES * u)
//              , step_notehead_width_px(STEP_NOTEHEAD_WIDTH_SPACES * u)
//              , step_notehead_padding_after_px(STEP_NOTEHEAD_PADDING_AFTER_SPACES * u)
//
//              , step_width_px(step_accidental_padding_before_px + step_accidental_width_px +
//                              step_notehead_padding_before_px + step_notehead_width_px +
//                              step_notehead_padding_after_px)
//
//              , step_notehead_x(1.5f * u)
//              , step_accidental_x(0.3f * u)
//
//              , step_ledger_line_before_px(0.3f * u)
//              , step_ledger_line_x(step_notehead_x - step_ledger_line_before_px)
//              , ledger_line_width_px(1.8f * u)
//              , ledger_line_thickness_px(2.0f * staff_line_thickness_px) {}
//
//
//    // public
//    float staff_space_px;
//    float half_staff_space_px;
//    float quarter_staff_space_px;
//    float staff_line_thickness_px;
//
//    float clef_padding_before_px;
//    float clef_padding_after_px;
//    float clef_width_px;
//    float clef_total_width_px;
//
//    float step_accidental_padding_before_px;
//    float step_accidental_width_px;
//    float step_notehead_padding_before_px;
//    float step_notehead_width_px;
//    float step_notehead_padding_after_px;
//
//    float step_width_px;
//
//    float step_notehead_x;
//    float step_accidental_x;
//
//    float step_ledger_line_before_px;
//    float step_ledger_line_x;
//    float ledger_line_width_px;
//    float ledger_line_thickness_px;
//};
//
//
//// ==============================================================================================
//
//class NoteView {
//public:
//    static const int REST = -1;
//
//    enum class Accidental {
//        dflat = -2
//        , flat = -1
//        , natural = 0
//        , sharp = 1
//        , dsharp = 2
//        , qflat = 0
//        , q3flat = 0
//        , qsharp = 0
//        , q3sharp = 0
//    };
//
//
//    NoteView(int base_pitch, Accidental accidental) : m_base_pitch(base_pitch), m_accidental(accidental) {}
//
//
//    static NoteView rest() {
//        return {REST, Accidental::natural};
//    }
//
//
//    int value() const { return m_base_pitch + static_cast<int>(m_accidental); }
//
//
//    int get_base_pitch() const { return m_base_pitch; }
//
//
//    bool is_rest() const {
//        return m_base_pitch == REST;
//    }
//
//
//    Accidental get_accidental() const { return m_accidental; }
//
//
//private:
//    int m_base_pitch;
//    Accidental m_accidental;
//};
//
//
//// ==============================================================================================
//
//class NoteSlider : public MultiSliderElement<int> {
//public:
//    // TODO: Handle paths properly
//    struct Images {
//        juce::Image rest = juce::ImageFileFormat::loadFrom(
//                juce::File("/Users/joakimborg/Pictures/Notation/4rest.png"));
//        juce::Image ottava = juce::ImageFileFormat::loadFrom(
//                juce::File("/Users/joakimborg/Pictures/Notation/8va.png"));
//        juce::Image flat = juce::ImageFileFormat::loadFrom(
//                juce::File("/Users/joakimborg/Pictures/Notation/flat.png"));
//        juce::Image natural = juce::ImageFileFormat::loadFrom(
//                juce::File("/Users/joakimborg/Pictures/Notation/natural.png"));
//        juce::Image sharp = juce::ImageFileFormat::loadFrom(
//                juce::File("/Users/joakimborg/Pictures/Notation/sharp.png"));
//        juce::Image notehead = juce::ImageFileFormat::loadFrom(
//                juce::File("/Users/joakimborg/Pictures/Notation/notehead.png"));
//    };
//
//
//    Voice<int> get_value() override {
//        return m_value;
//    }
//
//
//    void set_value(const Voice<int>& value) override {
//        m_value = value;
//    }
//
//
//    void paint(juce::Graphics& g) override {
//        g.drawFittedText(std::to_string(m_value.first_or(0)), getLocalBounds(), juce::Justification::centred, 1);
//    }
//
//
//protected:
//    bool state_changed(const MouseState<>& mouse_state) override {
//        if (mouse_state.is_down && !mouse_state.is_drag_editing) {
//            m_value = Voice<int>::singular(m_value.first_or(0) + 1);
//            return true;
//        }
//        return false;
//    }
//
//
//    void on_resized() override {
//
//    }
//
//
//private:
//    Voice<int> m_value;
//
//
//};
//
//
//// ==============================================================================================
//
//class ScoreSlider : public MultiSliderBase<int> {
//public:
//    explicit ScoreSlider(Sequence<Facet, int>& sequence) : MultiSliderBase<int>(sequence, MultiSliderBase<int>::RedrawOnChangeMode::single) {}
//
//    void paint(juce::Graphics &g) override {
//        g.fillAll(juce::Colours::rebeccapurple);
//    }
//
//protected:
//    std::unique_ptr<MultiSliderElement<int>> create_new_slider(const std::optional<Voice<int>>& initial_value) override {
//        return std::make_unique<NoteSlider>();
//    }
//
//
//    Margins get_margins() override {
//        return Margins{0, 0, 0, 0};
//    }
//
//
//    float get_header_width() override {
//        return 10;
//    }
//
//
//    Vec<float> slider_widths(
//            int total_multi_slider_area_width
//            , const Vec<std::unique_ptr<MultiSliderElement<int>>>& sliders
//            , const std::optional<std::pair<Voice<int>, std::size_t>>& insert_slider_value_and_index) override {
//
//        std::size_t num_sliders = sliders.size() + static_cast<std::size_t>(insert_slider_value_and_index.has_value());
//        float slider_width = static_cast<float>(total_multi_slider_area_width) / static_cast<float>(num_sliders);
//
//        return Vec<float>::repeated(num_sliders, slider_width);
//    }
//
//
////    void draw_background(juce::Graphics& g) override {
////        g.fillAll(juce::Colours::rebeccapurple);
////    }
//
//
////    void draw_header(juce::Graphics& g) override {
////        // TODO (not sure if this should be drawn or be a juce::Component (i.e. on_resize rather than draw).
////    }
//
//
//    void on_resize() override {
//        // TODO
//    }
//
//
//    void on_slider_value_changed() override {
//        // TODO
//    }
//};
//
//
//#endif //SERIALISTLOOPER_SCORE_SLIDER_H
