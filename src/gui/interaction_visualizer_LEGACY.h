
#ifndef JUCEGUIPLAYGROUND_INTERACTION_VISUALIZER_H
#define JUCEGUIPLAYGROUND_INTERACTION_VISUALIZER_H

#include <juce_gui_basics/juce_gui_basics.h>
#include "global_action_handler_LEGACY.h"
#include "key_state.h"

class InteractionVisualization_LEGACY : public juce::Component {
public:
    explicit InteractionVisualization_LEGACY(juce::Component& source_component)
            : m_source_component(source_component) {
        setWantsKeyboardFocus(false);
        setInterceptsMouseClicks(false, false);
    }


    virtual void update_state(bool mouse_is_over_component, Action* active_action) = 0;

protected:
    juce::Component& get_source_component() {
        return m_source_component;
    }


private:
    juce::Component& m_source_component;
};


// ==============================================================================================

class InteractionVisualizer_LEGACY
        : public juce::Component
          , public GlobalActionHandler::Listener
          , public GlobalKeyState::Listener {

public:
    explicit InteractionVisualizer_LEGACY(juce::Component& mouse_source_component
                                          , Vec<std::unique_ptr<InteractionVisualization_LEGACY>> visualizations)
            : m_mouse_source_component(mouse_source_component)
              , m_visualizations(std::move(visualizations)) {

        setWantsKeyboardFocus(false);
        setInterceptsMouseClicks(false, false);

        for (auto& visualization: m_visualizations) {
            addAndMakeVisible(*visualization);
        }

        m_mouse_source_component.addMouseListener(this, true);
        GlobalActionHandler::add_listener(*this);
        GlobalKeyState::add_listener(*this);
    }


    ~InteractionVisualizer_LEGACY() override {
        m_mouse_source_component.removeMouseListener(this);
        GlobalActionHandler::remove_listener(*this);
        GlobalKeyState::remove_listener(*this);
    }


    void resized() override {
        for (auto& visualization: m_visualizations) {
            visualization->setBounds(getLocalBounds());
        }
    }


    void add_visualization(std::unique_ptr<InteractionVisualization_LEGACY> v) {
        addAndMakeVisible(*v);
        m_visualizations.append(std::move(v));
        resized();
    }


    void add_visualization(Vec<std::unique_ptr<InteractionVisualization_LEGACY>> visualizations) {
        for (auto& v: visualizations) {
            add_visualization(std::move(v));
        }
    }


    void update_visualizations() {
        bool mouse_is_over = m_mouse_is_over_component || m_is_drag_and_dropping;

        for (auto& visualization: m_visualizations) {
            visualization->update_state(mouse_is_over, m_active_action);
        }
    }


    void set_drag_and_dropping(bool is_drag_and_dropping) {
        if (m_mouse_is_over_component != is_drag_and_dropping) {
            m_mouse_is_over_component = is_drag_and_dropping;
            update_visualizations();
        }
//        if (m_is_drag_and_dropping != is_drag_and_dropping) {
//            m_is_drag_and_dropping = is_drag_and_dropping;
//            if (m_is_drag_and_dropping) {
//                m_mouse_is_over_component = false;
//            }

//        }
    }


    void on_action_change(Action* action) override {
        m_active_action = action;
        update_visualizations();
    }


    void key_pressed() override {
        update_visualizations();
    }


    void key_released() override {
        update_visualizations();
    }


    void mouseEnter(const juce::MouseEvent&) override {
        m_mouse_is_over_component = true;
        update_visualizations();
    }


    void mouseExit(const juce::MouseEvent&) override {
        m_mouse_is_over_component = false;
        update_visualizations();
    }


private:
    juce::Component& m_mouse_source_component;


    Vec<std::unique_ptr<InteractionVisualization_LEGACY>> m_visualizations;

    bool m_mouse_is_over_component = false;
    bool m_is_drag_and_dropping = false;
    Action* m_active_action = nullptr;

};

#endif //JUCEGUIPLAYGROUND_INTERACTION_VISUALIZER_H
