
#ifndef SERIALISTLOOPER_MULTI_SLIDER_H
#define SERIALISTLOOPER_MULTI_SLIDER_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "state/generative_component.h"
#include "core/generatives/sequence.h"
#include "interaction_visualizer_LEGACY.h"
#include "mouse_state.h"
#include "keyboard_shortcuts.h"
#include "core/algo/facet.h"
#include "multi_slider_element.h"
#include "multi_slider_action.h"



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

class HeaderComponent : public juce::Component {
    // TODO
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

    virtual std::unique_ptr<MultiSliderElement<T>> create_new_slider(const Voice<T>& initial_value = {}) = 0;


    virtual Margins get_margins() {
        return {};
    }


    virtual float get_header_width() {
        return 0;
    }


    virtual RedrawOnChangeMode get_redraw_mode() {
        return RedrawOnChangeMode::single;
    }


    virtual std::optional<HeaderComponent> create_header() {
        return std::nullopt;
    }


    virtual Vec<int> slider_widths(const juce::Rectangle<int>& bounds
                                   , const Vec<std::reference_wrapper<MultiSliderElement<T>>>& slider_values) {
        if (slider_values.empty())
            return {};

        auto slider_width = bounds.getWidth() / static_cast<int>(slider_values.size());

        // rather add empty space to the right than stretch slider wider than its height
        slider_width = std::min(slider_width, bounds.getHeight());

        return Vec<int>::repeated(slider_values.size(), slider_width);
    }


    virtual void draw_background(juce::Graphics& g, juce::Rectangle<int> bounds) {
        g.setColour(SerialistLookAndFeel::OBJECT_BACKGROUND_C);
        g.fillRect(bounds);
        g.setColour(SerialistLookAndFeel::OBJECT_BORDER_C);
        g.drawRect(bounds);
    }


    virtual void on_slider_value_changed(const MultiSliderElement<T>&) {}

};

// ==============================================================================================



template<typename T>
class MultiSlider : public GenerativeComponent
                    , private GlobalKeyState::Listener
                    , private MultiSliderElement<T>::Listener
                    , private juce::ValueTree::Listener {
public:
    struct InsertSlider {
        std::unique_ptr<MultiSliderElement<T>> slider;
        std::size_t index;


        std::pair<Voice<T>, std::size_t> get_value_and_index() {
            return {slider->get_rendered_value(), index};
        }
    };


    MultiSlider(Sequence<Facet, T>& sequence, std::unique_ptr<MultiSliderConfig<T>> config)
            : m_sequence(sequence)
              , m_config(std::move(config)) {

        // TODO: Too complicated syntax: add a addListener function to ParameterHandler
        GlobalKeyState::add_listener(*this);
        m_sequence.get_parameter_handler().get_value_tree().addListener(this);


        for (auto& voice: m_sequence.get_values_raw()) {
            add_slider(voice);
        }

        // TODO
//        addAndMakeVisible(header);

    }


    ~MultiSlider() override {
        GlobalKeyState::remove_listener(*this);
        m_sequence.get_parameter_handler().get_value_tree().removeListener(this);
    }


    MultiSlider(const MultiSlider&) = delete;
    MultiSlider& operator=(const MultiSlider&) = delete;
    MultiSlider(MultiSlider&&) noexcept = default;
    MultiSlider& operator=(MultiSlider&&) noexcept = default;


    Generative& get_generative() override {
        return m_sequence;
    }


    void set_layout(int layout_id) override {
        (void) layout_id;
        // TODO
    }


    void paint(juce::Graphics& g) final {
        m_config->draw_background(g, getLocalBounds());
    }


    void resized() final {
        auto margins = m_config->get_margins();

        auto bounds = getLocalBounds();
        bounds.removeFromTop(margins.top);
        bounds.removeFromLeft(margins.left);
        bounds.removeFromRight(margins.right);
        bounds.removeFromBottom(margins.bottom);;


//        m_header.setBounds(bounds.removeFromLeft(get_header_width()));

        render_sliders(bounds);
    }


    void mouseEnter(const juce::MouseEvent& event) final {
        if (event.originalComponent != this) {
            std::cout << "Parent enter (FROM CHILD)"
            << "[is mouse over: " << isMouseOver() << "]"
            << "component at mouse (w, h): "
            << juce::Desktop::getInstance().findComponentAt(juce::Desktop::getMousePosition())->getWidth()
            << juce::Desktop::getInstance().findComponentAt(juce::Desktop::getMousePosition())->getHeight()
            << std::endl;

        } else {
            std::cout << "Parent enter" << std::endl;
        }
        m_mouse_state.mouse_enter(event);
    }


    void mouseMove(const juce::MouseEvent& event) final {
        m_mouse_state.mouse_move(event);
    }


    void mouseExit(const juce::MouseEvent& event) final {
        if (event.originalComponent != this) {
            std::cout << "Parent exit (FROM CHILD)" << std::endl;
        } else {
            std::cout << "Parent exit" << std::endl;
        }
        m_mouse_state.mouse_exit();
    }


    void mouseDown(const juce::MouseEvent& event) final {
        m_mouse_state.mouse_down(event);
        if (GlobalKeyState::is_down('Q')) {
            auto slider = m_config->create_new_slider();
            auto index = m_sliders.size(); // TODO: Should use mouse location
            begin_action(std::make_unique<MultiSliderInsert<T>>(index, std::move(slider)));
            resized();
        }
    }


    void mouseDrag(const juce::MouseEvent& event) final {
        m_mouse_state.mouse_drag(event);
    }


    void mouseUp(const juce::MouseEvent& event) final {
        m_mouse_state.mouse_up(event); // TODO: tempo
        if (m_ongoing_action) {
//            execute_action();
        }

//        bool mouse_is_currently_down = m_mouse_state.is_down;
//        m_mouse_state.mouse_up(event);
//
//        if (mouse_is_currently_down) {
//            trigger_action();
//        }
    }


    void key_pressed() override {
    }


    void key_released() override {}


    void modifier_keys_changed() override {

    }


private:
    void render_sliders(juce::Rectangle<int>& bounds) {
        if (m_ongoing_action) {
            for (auto& slider: m_ongoing_action->get_temporary_components()) {
                addAndMakeVisible(&slider.get());
            }

            // `rendered_sliders` are either added/made visible by above statement or previously through `add_slider`.
            auto rendered_sliders = m_ongoing_action->peek(m_sliders);
            auto slider_widths = m_config->slider_widths(bounds, rendered_sliders);

            for (std::size_t i = 0; i < slider_widths.size(); ++i) {
                rendered_sliders[i].get().setBounds(bounds.removeFromLeft(slider_widths[i]));
            }

        } else {
            auto slider_widths = m_config->slider_widths(bounds, MultiSliderAction<T>::to_reference(m_sliders));
            for (std::size_t i = 0; i < slider_widths.size(); ++i) {
                m_sliders[i]->setBounds(bounds.removeFromLeft(slider_widths[i]));
            }
        }
    }


    void slider_value_changed(MultiSliderElement<T>& slider) override {
        auto index = m_sliders.index([&slider](auto& s) { return s.get() == &slider; });
        assert(index);

        if (index) {
            m_sequence.set_value_at(*index, slider.get_actual_value());
        }
    }


    void slider_flagged_for_deletion(MultiSliderElement<T>& slider) override {}


    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override {
        // TODO
    }


//    void create_temporary_insert_slider() {
//        // TODO
//    }
//
//
//    void delete_temporary_insert_slider() {
//        // TODO
//    }
//
//
//    void finalize_temporary_insert_slider() {
//        // TODO
//    }


    void add_slider(const Voice<T>& initial_value = {}) {
        auto slider = m_config->create_new_slider(initial_value);
        slider->addMouseListener(this, true);
        addAndMakeVisible(*slider);
        slider->add_listener(*this);
        m_sliders.append(std::move(slider));
    }
//
//    void add_end_slider() {
//        m_end_slider = m_config->create_new_slider(std::nullopt);
//        addAndMakeVisible(*m_end_slider);
//        // TODO: Listener?
//    }


    void delete_slider() {
        // TODO
    }


    void begin_action(std::unique_ptr<MultiSliderAction<T>> action) {
        if (m_ongoing_action)
            cancel_action();

        m_ongoing_action = std::move(action);
        resized();
    }


    void modify_ongoing_action() {

    }


    void execute_action() {
        m_sliders = std::move(m_ongoing_action->execute());
        resized();

    }


    void cancel_action() {

    }


    Sequence<Facet, T>& m_sequence;
    std::unique_ptr<MultiSliderConfig<T>> m_config;

    Vec<std::unique_ptr<MultiSliderElement<T>>> m_sliders;
//    juce::Component& m_header;

    std::unique_ptr<MultiSliderAction<T>> m_ongoing_action = nullptr;

    MouseState<> m_mouse_state;

};


#endif //SERIALISTLOOPER_MULTI_SLIDER_H
