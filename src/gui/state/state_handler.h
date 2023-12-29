
#ifndef SERIALISTLOOPER_STATE_HANDLER_H
#define SERIALISTLOOPER_STATE_HANDLER_H

#include "key_state.h"
#include "state.h"
#include "state_condition.h"
#include "mouse_state.h"
#include "core/collections/vec.h"

class Stateful {
public:
    Stateful() = default;
    virtual ~Stateful() = default;
    Stateful(const Stateful&) = delete;
    Stateful& operator=(const Stateful&) = delete;
    Stateful(Stateful&&) noexcept = default;
    Stateful& operator=(Stateful&&) noexcept = default;

    virtual void update_state(const State& active_state) = 0;
};


// ==============================================================================================

struct TriggerableState {
    std::unique_ptr<Condition> condition;
    State state;
};


// ==============================================================================================

class StateHandler
        : public GlobalKeyState::Listener5
          , public juce::MouseListener
    // public GlobalMidiListener TODO: Add once implemented
    // public GlobalOscListener TODO: Add once implemented
{
public:
    StateHandler(StateHandler* parent
                 , juce::Component& mouse_source_component
                 , Vec<std::reference_wrapper<Stateful>> statefuls
                 , Vec<TriggerableState> triggerable_states = Vec<TriggerableState>()
                 , State initial_state = State::null_state())
            : m_parent(parent)
              , m_mouse_source_component(mouse_source_component)
              , m_statefuls(std::move(statefuls))
              , m_states(std::move(triggerable_states))
              , m_active_state(initial_state) {
        if (m_parent) {
            m_parent->add_child_handler(*this);
        }
        m_mouse_source_component.addMouseListener(this, true);
        GlobalKeyState::add_listener(*this);
    }


    StateHandler(StateHandler* parent
                 , juce::Component& mouse_source_component
                 , Stateful& stateful
                 , Vec<TriggerableState> triggerable_states = Vec<TriggerableState>()
                 , State initial_state = State::null_state())
            : StateHandler(parent
                           , mouse_source_component
                           , Vec<std::reference_wrapper<Stateful>>::singular(stateful)
                           , std::move(triggerable_states), initial_state) {}


    ~StateHandler() override {
        m_mouse_source_component.removeMouseListener(this);
        if (m_parent) {
            m_parent->remove_child_handler(*this);
        }
    }


    StateHandler(const StateHandler&) = delete;
    StateHandler& operator=(const StateHandler&) = delete;
    StateHandler(StateHandler&&) noexcept = delete;
    StateHandler& operator=(StateHandler&&) noexcept = delete;


    void mouseEnter(const juce::MouseEvent& event) override {
        bool is_directly_over = mouse_is_directly_over();

        // TODO: Not sure if the position of the mouse will be correct if passed from a child component
        // entering this component from outside
        if (!m_last_mouse_state.is_over_component()) {
            // entering registered child directly from outside
            if (!is_directly_over) {
                m_last_mouse_state.mouse_child_enter(event);
                update_state();

                // entering this component (or a non-registered child) from outside
            } else {
                m_last_mouse_state.mouse_enter(event);
                update_state();
            }

            // entering registered child from inside this component
        } else if (m_last_mouse_state.is_directly_over_component() && !is_directly_over) {
            m_last_mouse_state.mouse_child_enter(event);
            update_state();

        }
        // entering a non-registered component from a registered child does not need to be registered,
        // as this will first result in a mouseExit call


    }


    void mouseExit(const juce::MouseEvent&) override {
        bool is_over = mouse_is_over();

        // exit this component (from this component or any registered/unregistered child)
        if (!is_over) {
            m_last_mouse_state.mouse_exit();
            update_state();
        } else if (!m_last_mouse_state.is_directly_over_component() && mouse_is_directly_over()) {
            // exit a registered child into this component
            m_last_mouse_state.mouse_child_exit();
            update_state();
        }
    }


    void mouseDown(const juce::MouseEvent& event) override {
        // TODO: hide mouse could be passed here
        m_last_mouse_state.mouse_down(event);
        update_state();
    }


    void mouseDrag(const juce::MouseEvent& event) override {
        if (m_last_mouse_state.mouse_drag(event)) {
            update_state();
        }
    }


    void mouseUp(const juce::MouseEvent& event) override {
        // TODO: Restore mouse condition here
        m_last_mouse_state.mouse_up(event);
        update_state();
    }


    void modifier_keys_changed() override {
        if (m_last_mouse_state.is_over_component()) {
            update_state();
        }
    }


    void key_pressed() override {
        if (m_last_mouse_state.is_over_component()) {
            update_state();
        }
    }


    void key_released() override {
        if (m_last_mouse_state.is_over_component()) {
            update_state();
        }
    }



//    void mouseDoubleClick(const juce::MouseEvent& event) override {
//        // TODO
//    }

    // TODO: This is necesary to allow user to change/map the condition for triggering
//    void add_state(TriggerableState&& state) {
//        m_states.emplace_back(std::move(state));
//    }

//    void remove_state() // TODO: This is necessary if a key/condition needs to be reassigned


private:
    void update_state() {
        for (const auto& state: m_states) {
            if (state.condition->is_met()) {
                if (state.state != m_active_state) {
                    m_active_state = state.state;
                    notify();
                }
                return;
            }
        }

        if (!m_active_state.is_null()) {
            m_active_state = State::null_state();
            notify();
        }
    }


    void notify() const {
        for (const auto& stateful: m_statefuls) {
            stateful.get().update_state(m_active_state);
        }
    }


    void add_child_handler(StateHandler& child) {
        m_child_handlers.append(child);
    }


    void remove_child_handler(StateHandler& child) {
        m_child_handlers.remove([&child](const auto& handler) {
            return std::addressof(handler.get()) == std::addressof(child);
        });
    }


    /**
    * @return true if mouse is over the component or any of its child components
    */
    bool mouse_is_over() const {
        return m_mouse_source_component.isMouseOver(true);
    }


    /**
     * @return true if mouse is directly over the component or over a component that doesn't have a StateHandler
     */
    bool mouse_is_directly_over() const {
        return mouse_is_over() && !mouse_is_over_child();
    }


    /**
     * @return true if mouse is over a component that has a registered child StateHandler
     */
    bool mouse_is_over_child() const {
        return m_child_handlers.any([](const auto& handler) {
            return handler.get().mouse_is_over();
        });
    }


    StateHandler* m_parent;
    Vec<std::reference_wrapper<StateHandler>> m_child_handlers;


    juce::Component& m_mouse_source_component;
    Vec<std::reference_wrapper<Stateful>> m_statefuls;

    Vec<TriggerableState> m_states;

    MouseState m_last_mouse_state;
    State m_active_state;

};

#endif //SERIALISTLOOPER_STATE_HANDLER_H
