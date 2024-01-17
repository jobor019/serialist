
#ifndef SERIALISTLOOPER_INPUT_HANDLER_H
#define SERIALISTLOOPER_INPUT_HANDLER_H

#include <juce_gui_basics/juce_gui_basics.h>
#include "generative_component.h"
#include "key_state.h"
#include "drag_and_drop_LEGACY.h"
#include "input_mode.h"
#include "input_condition.h"
#include "input_mode_map.h"
#include "interaction/drag_and_drop/drag_and_drop.h"


class InputHandler
        : public GlobalKeyState::Listener
          , public juce::MouseListener
          , public DropListener {
public:
    static const int NO_STATE = -1;

    class Listener {
    public:
        Listener() = default;
        virtual ~Listener() = default;
        Listener(const Listener&) = delete;
        Listener& operator=(const Listener&) = delete;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;

        virtual void state_changed(InputMode* active_mode, int state) = 0;
    };


    InputHandler(InputHandler* parent
                 , juce::Component& mouse_source_component
                 , InputModeMap modes
                 , GlobalDragAndDropContainer& global_dnd_container
                 , Vec<std::reference_wrapper<Listener>> listeners
                 , const std::string& identifier = "")
            : m_parent(parent)
              , m_mouse_source_component(mouse_source_component)
              , m_modes(std::move(modes))
              , m_global_dnd_container(global_dnd_container)
              , m_drag_controller(&mouse_source_component, global_dnd_container)
              , m_listeners(std::move(listeners))
              , m_identifier(identifier) {
        if (m_parent) {
            m_parent->add_child_handler(*this);
        }
        m_mouse_source_component.addMouseListener(this, true);
        m_global_dnd_container.add_listener(*this);
        GlobalKeyState::add_listener(*this);

        if (update_active_mode()) {
            notify_listeners();
        }
    }


    ~InputHandler() override {
        m_mouse_source_component.removeMouseListener(this);
        m_global_dnd_container.remove_listener(*this);
        GlobalKeyState::remove_listener(*this);
    }


    InputHandler(const InputHandler&) = delete;
    InputHandler& operator=(const InputHandler&) = delete;
    InputHandler(InputHandler&&) noexcept = delete;
    InputHandler& operator=(InputHandler&&) noexcept = delete;


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


    juce::Component* component_under_mouse() const {
        if (!mouse_is_over()) {
            return nullptr;
        }

        for (const auto& handler: m_child_handlers) {
            if (handler.get().mouse_is_over()) {
                return &handler.get().m_mouse_source_component;
            }
        }

        return &m_mouse_source_component;
    }


    void mouseEnter(const juce::MouseEvent& event) override {
        std::cout << "¤¤¤" << m_identifier << "::Event - MouseEnter\n";
        bool is_directly_over = mouse_is_directly_over();

        // TODO: Not sure if the position of the mouse will be correct if passed from a child component
        if (!m_last_mouse_state.is_over_component()) {
            // entering this component from outside

            if (!is_directly_over) {
                // entering registered child directly from outside
                m_last_mouse_state.mouse_child_enter(event);
                if (mouse_changed(false)) {
                    notify_listeners();
                }


            } else {
                // entering this component (or a non-registered child) from outside
                m_last_mouse_state.mouse_enter(event);
                if (mouse_changed(false)) {
                    notify_listeners();
                }
            }


        } else if (m_last_mouse_state.is_directly_over_component() && !is_directly_over) {
            // entering registered child from inside this component
            m_last_mouse_state.mouse_child_enter(event);
            if (mouse_changed(false)) {
                notify_listeners();
            }

        }
        // entering a non-registered component from a registered child does not need to be registered,
        // as this will first result in a mouseExit call


    }


    void mouseMove(const juce::MouseEvent& event) override {
        m_last_mouse_state.mouse_move(event);
        if (mouse_changed(true)) {
            notify_listeners();
        }
    }


    void mouseExit(const juce::MouseEvent&) override {
        std::cout << "¤¤¤" << m_identifier << "::Event - MouseExit\n";
        bool is_over = mouse_is_over();

        // exit this component (from this component or any registered/unregistered child)
        if (!is_over) {
            m_last_mouse_state.mouse_exit();
            if (mouse_changed(false)) {
                notify_listeners();
            }
        } else if (!m_last_mouse_state.is_directly_over_component() && mouse_is_directly_over()) {
            // exit a registered child into this component
            m_last_mouse_state.mouse_child_exit();
            if (mouse_changed(false)) {
                notify_listeners();
            }
        }
    }


    void mouseDown(const juce::MouseEvent& event) override {
        std::cout << "¤¤¤" <<  m_identifier << "::Event - MouseDown\n";
        auto hide = m_active_mode && m_active_mode->get_drag_behaviour() == DragBehaviour::hide_and_restore;
        m_last_mouse_state.mouse_down(event, hide);
        if (mouse_changed(false)) {
            notify_listeners();
        }
    }


    void mouseDrag(const juce::MouseEvent& event) override {
        // TODO: We probably need to implement a MouseClickEvent and determine whether a mouseUp + mouseDown
        //      was a click or a drag (to avoid minimal accidental movements during click turning into drags)

        // TODO: This is also problematic for drag starts, as it doesn't check whether it's directly over the component.
        //       To properly implement this, we need to check all registered children and see if any component under
        //       the mouse is in a mode which actively handles drag starts. If so, it should intercept it.

        // an ongoing drag has previously been cancelled. No further actions until mouse button is released
        if (m_drag_cancelled) {
            return;
        }

        // ongoing drag edit within this component
        if (m_last_mouse_state.is_drag_editing) {
            m_last_mouse_state.mouse_drag_edit(event);
            if (mouse_changed(true)) {
                notify_listeners();
            }
            return;
        }


        // ongoing drag and drop from this component
        if (m_last_mouse_state.is_dragging_from) {
            if (mouse_changed(true)) {
                notify_listeners();
            }
            return;
        }

        // new drag edit in this component without any active state or with default drag edit behaviour
        if (m_active_mode && m_active_mode->get_drag_behaviour() == DragBehaviour::drag_edit) {
            m_last_mouse_state.mouse_drag_edit(event);
            if (mouse_changed(false)) {
                notify_listeners();
            }
            return;
        }

        // new drag and drop from this component
        if (m_active_mode && m_active_mode->get_drag_behaviour() == DragBehaviour::drag_and_drop) {
            start_drag_from(event);
            if (mouse_changed(false)) {
                notify_listeners();
            }
            return;
        }

        // DragBehaviour::HideAndRestoreCursor is handled on mouseDown and mouseUp, not mouseDrag
    }


    void mouseUp(const juce::MouseEvent& event) override {
        std::cout << "¤¤¤" << m_identifier << "::Event - MouseUp\n";
        if (m_last_mouse_state.is_dragging_from) {
            finalize_drag_from();
        } else {
            auto restore = m_active_mode && m_active_mode->get_drag_behaviour() == DragBehaviour::hide_and_restore;
            m_last_mouse_state.mouse_up(event, restore);
            if (mouse_changed(false)) {
                notify_listeners();
            }
        }

        reset_drag_cancel();
    }


    bool interested_in(const DragInfo& source) override {
        return m_active_mode && m_active_mode->supports_drop_from(source);
    }


    /**
     * @note exact mouse position is currently not used
     */
    void drop_enter(const DragInfo& source, const juce::MouseEvent&) override {
        assert(interested_in(source)); // Transitive from caller calling interested_in

        // Note: GlobalDragAndDropController will manage relation parent/child
        //       so there's no need to check is_directly_over
        std::cout << m_identifier <<"::Drop enter\n";
        m_last_mouse_state.drop_enter();
        if (mouse_changed(false)) {
            notify_listeners();
        }
    }


    void drop_exit(const DragInfo&) override {
        std::cout << m_identifier << "::Drop exit\n";
        m_last_mouse_state.drop_exit();
        if (mouse_changed(false)) {
            notify_listeners();
        }
    }


    void drop_move(const DragInfo&, const juce::MouseEvent&) override {
        // Unused for now
    }


    void item_dropped(const DragInfo& source) override {
        assert(interested_in(source)); // Transitive from caller calling interested_in

        m_last_mouse_state.drop_exit();
        if (update_state(m_active_mode->item_dropped(source))) {
            notify_listeners();
        }
    }


    void external_dnd_action_started(const DragInfo& source) override {
        assert(interested_in(source)); // Transitive from caller calling interested_in

        m_last_mouse_state.external_drag_start();
        std::cout << m_identifier << "::External drag start\n";
        if (mouse_changed(false)) {
            notify_listeners();
        }
    }


    void external_dnd_action_ended(const DragInfo& source) override {
        assert(interested_in(source)); // Transitive from caller calling interested_in

        m_last_mouse_state.external_drag_end();
        std::cout << m_identifier << "::External drag end\n";
        if (mouse_changed(false)) {
            notify_listeners();
        }
    }


//  TODO REMOVE I THINK
//    void end_drag_from() {
//        assert(m_active_mode); // Transitive from caller calling interested_in
//        m_last_mouse_state.drop_exit();
//        if (mouse_changed(false)) {
//            notify_listeners();
//        }
//    }


    void key_pressed() override {
        if (update_active_mode()) {
            notify_listeners();
        } else {
            // TODO: Check if any triggers are registered for given key_press
        }
    }


    void key_released() override {
        if (update_active_mode()) {
            notify_listeners();
        } else {
            // TODO: Check if any triggers are registered for given key_release
        }
    }


    void modifier_keys_changed() override {
        // TODO
    }


    GlobalDragAndDropContainer& get_global_dnd_container() const {
        return m_global_dnd_container;
    }


private:
    void start_drag_from(const juce::MouseEvent& mouse_event) {
        assert(m_active_mode);
        assert(m_active_mode->get_drag_behaviour() == DragBehaviour::drag_and_drop);

        m_drag_controller.start_drag(m_active_mode->get_drag_info(), mouse_event, m_active_mode->get_drag_image());
        m_last_mouse_state.drag_start();

        if (mouse_changed(false)) {
            notify_listeners();
        }
    }


    void finalize_drag_from() {
        assert(m_active_mode);
        assert(m_active_mode->get_drag_behaviour() == DragBehaviour::drag_and_drop);

        m_drag_controller.finalize_drag();
        m_last_mouse_state.drag_end();

        if (mouse_changed(false)) {
            notify_listeners();
        }
    }


    void cancel_all_drag_actions() {
        std::cout << m_identifier << "::CANCELLING ALL DND\n";
        m_drag_controller.cancel_drag();

        if (m_last_mouse_state.is_dragging())
            m_drag_cancelled = true;

        m_last_mouse_state.reset_drag_state();

    }


    void reset_drag_cancel() {
        m_drag_cancelled = false;
    }


    /**
     * @return true if the active mode has changed
     */
    bool update_active_mode() {
        if (auto* active_mode = m_modes.get_active(); active_mode != m_active_mode) {

            if (m_active_mode) {
                cancel_all_drag_actions();
                m_active_mode->mode_deactivated();
                m_active_mode->reset();
            }

            m_active_mode = active_mode;

            if (m_active_mode) {
                m_active_mode->mode_activated();
                mouse_changed(false);
                std::cout << m_identifier << "::new mode\n";
            } else {
                m_last_state = NO_STATE;
                std::cout << m_identifier << "::no mode\n";
            }

            return true;
        }

        return false;
    }


    /**
     * @return true if the state of the active mode has changed
     */
    bool mouse_changed(bool only_position_changed) {
        if (m_active_mode) {
            if (only_position_changed) {
                return update_state(m_active_mode->mouse_position_changed(m_last_mouse_state));
            } else {
                return update_state(m_active_mode->mouse_state_changed(m_last_mouse_state));
            }
        }

        return false;
    }


    /**
     * @return true if the state of the active mode has changed
     */
    bool update_state(std::optional<int> new_state) {
        if (new_state && new_state != m_last_state) {
            m_last_state = *new_state;
            std::cout << m_identifier << "::new state: " << m_last_state << std::endl;
            return true;
        }
        return false;
    }


    void notify_listeners() {
        for (const auto& listener: m_listeners) {
            listener.get().state_changed(m_active_mode, m_last_state);
        }
    }


    void add_child_handler(InputHandler& child) {
        m_child_handlers.append(child);
    }


    void remove_child_handler(InputHandler& child) noexcept {
        m_child_handlers.remove([&child](const auto& handler) {
            return std::addressof(handler.get()) == std::addressof(child);
        });
    }


    InputHandler* m_parent;
    Vec<std::reference_wrapper<InputHandler>> m_child_handlers;

    juce::Component& m_mouse_source_component;

    InputModeMap m_modes;

    GlobalDragAndDropContainer& m_global_dnd_container;
    DragController m_drag_controller;

    InputMode* m_active_mode = nullptr;
    int m_last_state = NO_STATE;

    Vec<std::reference_wrapper<Listener>> m_listeners;

    MouseState m_last_mouse_state;

    std::string m_identifier;

    bool m_drag_cancelled = false;
};


#endif //SERIALISTLOOPER_INPUT_HANDLER_H
