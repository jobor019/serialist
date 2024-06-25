
#ifndef SERIALISTLOOPER_MULTI_SLIDER_LEGACY_H
#define SERIALISTLOOPER_MULTI_SLIDER_LEGACY_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "state/generative_component.h"
#include "core/generatives/sequence.h"
#include "mouse_state.h"
#include "keyboard_shortcuts.h"
#include "core/algo/facet.h"
#include "multi_slider_element.h"
#include "multi_slider_action_LEGACY.h"
#include "bases/widget_stereotypes.h"



// ==============================================================================================

struct Margins {
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;
};

enum class RedrawOnChangeMode {
    all = 0
    , single = 1
    , trailing = 2
    , leading = 3
};


// ==============================================================================================

class MultiSliderStates {
public:
    inline static const State Insert{ModuleIds::MultiSliderWidget, 1};
    inline static const State Delete{ModuleIds::MultiSliderWidget, 2};
    inline static const State Move{ModuleIds::MultiSliderWidget, 3};
    inline static const State Duplicate{ModuleIds::MultiSliderWidget, 4};
};


// ==============================================================================================

//
//struct MultiSliderInsert {
//    std::size_t index;
//};
//
//
//struct MultiSliderDelete {
//    std::size_t index;
//};
//
//
//struct MultiSliderMove {
//    std::size_t from;
//    std::size_t to;
//};
//
//
//struct MultiSliderDuplicate {
//    std::size_t from;
//    std::size_t to;
//};
//
//
//struct MultiSliderAction {
//    using ActionType = std::variant<MultiSliderInsert, MultiSliderDelete, MultiSliderMove, MultiSliderDuplicate>;
//
//
//    explicit MultiSliderAction(ActionType a) : action(a) {}
//
//
//    template<typename T>
//    bool is() const noexcept {
//        return std::holds_alternative<T>(action);
//    }
//
//
//    template<typename T>
//    T as() const {
//        return std::get<T>(action);
//    }
//
//
//    State associated_state() const {
//        if (std::holds_alternative<MultiSliderInsert>(action)) {
//            return MultiSliderStates::Insert;
//        } else if (std::holds_alternative<MultiSliderDelete>(action)) {
//            return MultiSliderStates::Delete;
//        } else if (std::holds_alternative<MultiSliderMove>(action)) {
//            return MultiSliderStates::Move;
//        } else if (std::holds_alternative<MultiSliderDuplicate>(action)) {
//            return MultiSliderStates::Duplicate;
//        }
//
//        assert(false);
//    }
//
//
//    ActionType action;
//};


// ==============================================================================================

class SliderGapHighlight : public juce::Component {

};



// ==============================================================================================

template<typename T>
class MultiSliderConfig {
public:
    MultiSliderConfig() = default;
    virtual ~MultiSliderConfig() = default;
    MultiSliderConfig(const MultiSliderConfig&) = delete;
    MultiSliderConfig& operator=(const MultiSliderConfig&) = delete;
    MultiSliderConfig(MultiSliderConfig&&) noexcept = default;
    MultiSliderConfig& operator=(MultiSliderConfig&&) noexcept = default;

    virtual std::unique_ptr<MultiSliderElement<T>> create_new_slider() = 0;


    virtual Margins get_margins() {
        return {};
    }


    virtual float get_header_width() {
        return 0;
    }


    virtual float get_footer_width() {
        return 0;
    }


    virtual RedrawOnChangeMode get_redraw_mode() {
        return RedrawOnChangeMode::single;
    }


    virtual std::optional<juce::Component> create_header() {
        return std::nullopt;
    }


    virtual std::optional<juce::Component> create_footer() {
        return std::nullopt;
    }


    virtual Vec<int> get_slider_widths(const juce::Rectangle<int>& bounds
                                   , const Vec<std::unique_ptr<MultiSliderElement<T>>>& slider_values) {
        return Vec<int>::repeated(slider_values.size(), default_width(bounds, slider_values));
    }


    virtual juce::Rectangle<int> get_bounds_of(const MultiSliderElement<T>& mse
                                           , const juce::Rectangle<int>& bounds
                                           , const Vec<std::unique_ptr<MultiSliderElement<T>>>& sliders) {
        (void) mse;
        return default_width(bounds, sliders);
    }


    virtual void draw_background(juce::Graphics& g, juce::Rectangle<int> bounds) {
        g.setColour(SerialistLookAndFeel::OBJECT_BACKGROUND_C);
        g.fillRect(bounds);
        g.setColour(SerialistLookAndFeel::OBJECT_BORDER_C);
        g.drawRect(bounds);
    }


    virtual void on_slider_value_changed(const MultiSliderElement<T>&) {}


protected:
    int default_width(const juce::Rectangle<int>& bounds
                      , const Vec<std::unique_ptr<MultiSliderElement<T>>>& slider_values) {
        if (slider_values.empty())
            return 0;

        auto slider_width = bounds.getWidth() / static_cast<int>(slider_values.size());

        // rather add empty space to the right than stretch slider wider than its height
        slider_width = std::min(slider_width, bounds.getHeight());
    }

};


// ==============================================================================================

template<typename T>
class MultiSliderAction {
public:
    MultiSliderAction() = default;
    virtual ~MultiSliderAction() = default;
    MultiSliderAction(const MultiSliderAction&) = delete;
    MultiSliderAction& operator=(const MultiSliderAction&) = delete;
    MultiSliderAction(MultiSliderAction&&) noexcept = default;
    MultiSliderAction& operator=(MultiSliderAction&&) noexcept = default;

    virtual void apply(Voices<T>& voices
                       , Vec<MultiSliderElement<T>>& sliders
                       , const MultiSliderConfig<T>& config) const = 0;

    virtual void
    render(juce::Component& slider_border, juce::Component& slider_gap, Vec<MultiSliderElement<T>>& elements) = 0;

    virtual State& associated_state() const = 0;
};

template<typename T>
class MultiSliderInsert : public MultiSliderAction<T> {
public:
    explicit MultiSliderInsert(std::size_t index) : m_index(index) {}


    ~MultiSliderInsert() override {
        m_gap_highlight.setVisible(false);
    }


    void apply(Voices<T>& voices
               , Vec<MultiSliderElement<T>>& sliders
               , const MultiSliderConfig<T>& config) const override {
        auto mse = config.create_new_slider();
        voices.insert(m_index, mse->get_value());
        sliders.add(std::move(mse));
    }


    void render(    juce::Rectangle<int>& slider_bounds
                , const Vec<MultiSliderElement<T>>& mses
                , const MultiSliderConfig<T>& config) override {
        m_gap_highlight.setVisible(true);

        int insert_position_x;

        if (mses.empty()) {
            insert_position_x = 0;
        } else if (m_index >= mses.size()) {
            // TODO: Invalid syntax + bad bounds in parent, but this would be the general idea
            insert_position_x = config.get_bounds_of(mses.back(), slider_bounds, mses).getRight();
        } else {
            insert_position_x = config.get_bounds_of(mses[m_index], slider_bounds, mses).getRight();
        }

        m_gap_highlight.setBounds(slider_bounds.removeFromLeft(insert_position_x)); // TODO: Something like this
    }


    const State& associated_state() const override {
        return m_state;
    }


private:
    std::size_t m_index;
    State m_state = MultiSliderStates::Insert;
    SliderGapHighlight& m_gap_highlight;
};


template<typename T>
class MultiSliderDelete : public MultiSliderAction<T> {
public:
private:
    std::size_t m_index;
    State m_state = MultiSliderStates::Delete;
//    SliderBorderHighlight& m_border_highlight;
};

template<typename T>
class MultiSliderMove : public MultiSliderAction<T> {
public:
private:
    std::size_t m_from;
    std::size_t m_to;
    State m_state = MultiSliderStates::Move;
    SliderGapHighlight& m_gap_highlight;
};


// ==============================================================================================

template<typename T>
class MultiSliderWidget : public WidgetBase
                          , private MultiSliderElement<T>::Listener
                          , private juce::ValueTree::Listener {
public:
    MultiSliderWidget(Sequence<Facet, T>& sequence
                      , StateHandler& parent_state_handler
                      , std::unique_ptr<MultiSliderConfig<T>> config)
            : WidgetBase(sequence, parent_state_handler)
              , m_sequence(sequence)
              , m_config(std::move(config))
              , m_header(m_config->create_header())
              , m_footer(m_config->create_footer()) {

        m_sequence.get_parameter_handler().get_value_tree().addListener(this);
        for (auto& voice: m_sequence.get_values_raw()) {
            insert_slider(voice);
        }

        if (m_header)
            addAndMakeVisible(*m_header);

        if (m_footer)
            addAndMakeVisible(*m_footer);
    }


    ~MultiSliderWidget() override {
        m_sequence.get_parameter_handler().get_value_tree().removeListener(this);
    }


    MultiSliderWidget(const MultiSliderWidget&) = delete;
    MultiSliderWidget& operator=(const MultiSliderWidget&) = delete;
    MultiSliderWidget(MultiSliderWidget&&) noexcept = default;
    MultiSliderWidget& operator=(MultiSliderWidget&&) noexcept = default;


    void set_layout(int layout_id) override {
        (void) layout_id;
        // TODO
    }


    void paint(juce::Graphics& g) final {
        m_config->draw_background(g, getLocalBounds());
    }


    void on_resized(juce::Rectangle<int>& bounds) final {
        auto margins = m_config->get_margins();

        bounds.removeFromTop(margins.top);
        bounds.removeFromLeft(margins.left);
        bounds.removeFromRight(margins.right);
        bounds.removeFromBottom(margins.bottom);

        render_header(bounds);
        render_sliders(bounds);
        render_footer(bounds);
    }


    void state_changed(const State& active_state, const MouseState& mouse_state) override {
        // state is irrelevant for this component: ignore & cancel any ongoing action
        if (active_state.get_module_id() != ModuleIds::MultiSliderWidget
            || active_state == States::Default
            || active_state == States::NoInteraction) {

            if (m_ongoing_action) {
                cancel_action(active_state, mouse_state);
                resized();
            }
            return;
        }

        bool changed = false;

        if (m_ongoing_action && m_ongoing_action->associated_state() != active_state)
            changed = changed || cancel_action(active_state, mouse_state);

        if (mouse_state.was_just_clicked()) {
            changed = changed || begin_action(active_state, mouse_state);
        } else if (mouse_state.is_drag_editing) {
            changed = changed || update_action(active_state, mouse_state);
        } else if (mouse_state.was_just_released()) {
            changed = changed || finalize_action(active_state, mouse_state);
        }

        if (changed) {
            resized();
        }
    }


private:
    bool begin_action(const State& active_state, const MouseState& mouse_state) {
        if (m_ongoing_action) {
            cancel_action(active_state, mouse_state);
        }
    }


    bool update_action(const State& active_state, const MouseState& mouse_state) {

    }


    bool finalize_action(const State& active_state, const MouseState& mouse_state) {

    }


    bool cancel_action(const State& active_state, const MouseState& mouse_state) {

    }


    void render_header(juce::Rectangle<int>& bounds) {
        // TODO
    }


    void render_sliders(juce::Rectangle<int>& bounds) {
        auto slider_widths = m_config->slider_widths(bounds, m_sliders);
        for (std::size_t i = 0; i < slider_widths.size(); ++i) {
            m_sliders[i]->setBounds(bounds.removeFromLeft(slider_widths[i]));
        }

        if (m_ongoing_action) {
            if (m_ongoing_action->is<MultiSliderInsert>()) {
                m_gap_highlight.setVisible(true);
            } else if (m_ongoing_action->is<MultiSliderDelete>())
        }
    }


    void render_footer(juce::Rectangle<int>& bounds) {
        // TODO
    }


    void slider_value_changed(MultiSliderElement<T>& slider) override {
        auto index = m_sliders.index([&slider](auto& s) { return s.get() == &slider; });
        assert(index);

        if (index) {
            m_sequence.set_value_at(*index, slider.get_actual_value());
        }
    }


    void slider_flagged_for_deletion(MultiSliderElement<T>& slider) override {
        // TODO
    }


    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override {
        // TODO
    }


    void insert_slider() {
        auto slider = m_config->create_new_slider();
        slider->addMouseListener(this, true);
        addAndMakeVisible(*slider);
        slider->add_listener(*this);
        m_sliders.append(std::move(slider));
    }


    void delete_slider() {

    }


    void move_slider() {

    }


    void duplicate_slider() {

    }


    Sequence<Facet, T>& m_sequence;
    std::unique_ptr<MultiSliderConfig<T>> m_config;

    Vec<std::unique_ptr<MultiSliderElement<T>>> m_sliders;
    std::optional<juce::Component> m_header;
    std::optional<juce::Component> m_footer;

//    juce::Component& m_header;

    std::optional<MultiSliderAction> m_ongoing_action = std::nullopt;

    SliderGapHighlight m_gap_highlight;


};


#endif //SERIALISTLOOPER_MULTI_SLIDER_LEGACY_H
