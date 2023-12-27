
#ifndef SERIALISTLOOPER_MULTI_SLIDER_ELEMENT_H
#define SERIALISTLOOPER_MULTI_SLIDER_ELEMENT_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "key_state.h"
#include "interaction_visualizer_LEGACY.h"
#include "core/collections/vec.h"
#include "core/collections/voices.h"
#include "mouse_state.h"
#include "keyboard_shortcuts.h"


template<typename T>
class MultiSliderElement : public juce::Component
                           , public GlobalKeyState::Listener {
public:

    class Listener {
    public:
        Listener() = default;
        virtual ~Listener() = default;
        Listener(const Listener&) = delete;
        Listener& operator=(const Listener&) = delete;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;

        virtual void slider_value_changed(MultiSliderElement<T>& slider) = 0;
        virtual void slider_flagged_for_deletion(MultiSliderElement<T>& slider) = 0;
    };


    explicit MultiSliderElement(Vec<std::unique_ptr<InteractionVisualization>> extra_visualizations = {}
                                , bool restore_mouse_on_drag_end = false)
            : m_interaction_visualizer(*this, default_visualizations())
              , m_restore_mouse_on_drag_end(restore_mouse_on_drag_end) {
        m_interaction_visualizer.add_visualization(std::move(extra_visualizations));
        GlobalKeyState::add_listener(*this);

        addAndMakeVisible(m_interaction_visualizer);

    }


    ~MultiSliderElement() override {
        GlobalKeyState::remove_listener(*this);
    }


    MultiSliderElement(const MultiSliderElement&) = delete;
    MultiSliderElement& operator=(const MultiSliderElement&) = delete;
    MultiSliderElement(MultiSliderElement&&) noexcept = default;
    MultiSliderElement& operator=(MultiSliderElement&&) noexcept = default;


    Vec<std::unique_ptr<InteractionVisualization>> default_visualizations() {
        return {}; // TODO: mouseover(border), remove(borderandfill), maybe NEW(borderandfill) for insert slider
    }


    virtual const Voice<T>& get_rendered_value() const = 0;
    virtual const Voice<T>& get_actual_value() const = 0;
    virtual void set_value(const Voice<T>& value) = 0;


    void add_listener(Listener& listener) {
        m_listeners.add(&listener);
    }


    void remove_listener(Listener& listener) {
        m_listeners.remove(&listener);
    }


    void resized() final {
        m_interaction_visualizer.setBounds(getLocalBounds());
        on_resized();
    }


    void mouseEnter(const juce::MouseEvent& event) final {
        std::cout << "CHILD enter\n";
        m_mouse_state.mouse_enter(event);
        if (update_state(m_mouse_state)) {
            repaint();
            notify();
        }
    }


    void mouseMove(const juce::MouseEvent& event) final {
        m_mouse_state.mouse_move(event);
        if (update_state(m_mouse_state)) {
            repaint();
            notify();
        }
    }


    void mouseExit(const juce::MouseEvent& event) final {
        std::cout << "CHILD (non-simulated) exit\n";
        m_mouse_state.mouse_exit();
        if (update_state(m_mouse_state)) {
            repaint();
            notify();
        }
    }


    void mouseDown(const juce::MouseEvent& event) final {
        m_mouse_state.mouse_down(event);
        if (GlobalKeyState::is_down(delete_hotkey())) {
            // TODO: This is probably not a good strategy. We should register an ongoing Action, which
            //  - is registered on mouseDown if given key is down
            //  - triggers on mouseUp unless interrupted
            //  - is interrupted if key is released or mouseExit is registered
            flag_for_deletion();
        } else if (update_state(m_mouse_state)) {
            repaint();
            notify();
        }
    }


    void mouseDrag(const juce::MouseEvent& event) final {
        m_mouse_state.mouse_drag(event);
        if (update_state(m_mouse_state)) {
            repaint();
            notify();
        }
    }


    void mouseUp(const juce::MouseEvent& event) final {
        m_mouse_state.mouse_up(event, m_restore_mouse_on_drag_end);
        if (update_state(m_mouse_state)) {
            repaint();
            notify();
        }
    }


    void key_pressed() final {
        // TODO Temp
        if (GlobalKeyState::is_down_exclusive('Q')) {
            std::cout << "CHILD (simulated) exit + intercept: false\n";
            m_mouse_state.mouse_exit();
        }

        if (m_mouse_state.is_over_component() && update_state(m_mouse_state)) {
            repaint();
            notify();
        }
    }


    void key_released() final {
        if (!GlobalKeyState::is_down_exclusive('Q')) {
            std::cout << "CHILD intercept: true\n";
        }
        if (m_mouse_state.is_over_component() && update_state(m_mouse_state)) {
            repaint();
            notify();
        }
    }


    void modifier_keys_changed() final {
        if (m_mouse_state.is_over_component() && update_state(m_mouse_state)) {
            repaint();
            notify();
        }
    }


protected:
    /**
     * @return true if the value was changed
     */
    virtual bool update_state(const MouseState<>& mouse_state) = 0;


//    virtual void draw_background(juce::Graphics&) {}
//    virtual void draw_value(juce::Graphics& g, bool has_mouseover) = 0;
//    virtual void draw_mouseover(juce::Graphics& g) = 0;

    virtual void on_resized() = 0;


    virtual int delete_hotkey() {
        return MultiSliderKeyboardShortcuts::DELETE_SLIDER;
    }


    void flag_for_deletion() {
        m_listeners.call(([this](Listener& l) { l.slider_flagged_for_deletion(*this); }));
    }


    const MouseState<>& get_mouse_state() const { return m_mouse_state; }


private:
    void notify() {
        m_listeners.call(([this](Listener& l) { l.slider_value_changed(*this); }));
    }


//    virtual void draw_highlights(juce::Graphics& g) { }


    InteractionVisualizer m_interaction_visualizer;
    bool m_restore_mouse_on_drag_end;

    MouseState<> m_mouse_state;

    juce::ListenerList<Listener> m_listeners;
};
#endif //SERIALISTLOOPER_MULTI_SLIDER_ELEMENT_H
