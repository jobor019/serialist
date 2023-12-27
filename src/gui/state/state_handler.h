
#ifndef SERIALISTLOOPER_STATE_HANDLER_H
#define SERIALISTLOOPER_STATE_HANDLER_H

#include "key_state.h"
#include "state.h"
#include "state_condition.h"
#include "mouse_state.h"

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

template<int DragVelocitySize = 5>
class StateHandler
        : public GlobalKeyState::Listener
          , public juce::MouseListener
    // public GlobalMidiListener TODO: Add once implemented
    // public GlobalOscListener TODO: Add once implemented
{
public:
    StateHandler(juce::Component& mouse_source_component
                 , std::vector<std::reference_wrapper<Stateful>> statefuls
                 , std::vector<TriggerableState> triggerable_states
                 , State initial_state = State::null_state())
            : m_mouse_source_component(mouse_source_component)
              , m_statefuls(std::move(statefuls))
              , m_states(std::move(triggerable_states))
              , m_active_state(initial_state) {
        m_mouse_source_component.addMouseListener(this, true);
        GlobalKeyState::add_listener(*this);
    }


    ~StateHandler() override {
        m_mouse_source_component.removeMouseListener(this);
    }


    StateHandler(const StateHandler&) = default;
    StateHandler& operator=(const StateHandler&) = default;
    StateHandler(StateHandler&&) noexcept = default;
    StateHandler& operator=(StateHandler&&) noexcept = default;


    void mouseEnter(const juce::MouseEvent& event) override {
        // TODO: Not sure if the position of the mouse will be correct if passed from a child component
        m_mouse_state.mouse_enter(event);
        update_state();
    }


    void mouseExit(const juce::MouseEvent&) override {
        if (!m_mouse_source_component.isMouseOver(true)) {
            m_mouse_state.mouse_exit();
            update_state();
        }
    }


    void mouseDown(const juce::MouseEvent& event) override {
        // TODO: hide mouse could be passed here
        m_mouse_state.mouse_down(event);
        update_state();
    }


    void mouseDrag(const juce::MouseEvent& event) override {
        if (m_mouse_state.mouse_drag(event)) {
            update_state();
        }
    }


    void mouseUp(const juce::MouseEvent& event) override {
        // TODO: Restore mouse condition here
        m_mouse_state.mouse_up(event);
        update_state();
    }


    void modifier_keys_changed() override {
        if (m_mouse_state.is_over_component()) {
            update_state();
        }
    }


    void key_pressed() override {
        if (m_mouse_state.is_over_component()) {
            update_state();
        }
    }


    void key_released() override {
        if (m_mouse_state.is_over_component()) {
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


    // TODO: Not sure if this is needed: the MouseListener is already listening to all its children,
    //       so there should be no need to inform the parent of mouse actions
    // StateHandler* m_parent;

    juce::Component& m_mouse_source_component;
    std::vector<std::reference_wrapper<Stateful>> m_statefuls;

    std::vector<TriggerableState> m_states;

    MouseState<DragVelocitySize> m_mouse_state;
    State m_active_state;

};

#endif //SERIALISTLOOPER_STATE_HANDLER_H
