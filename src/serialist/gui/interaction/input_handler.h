#ifndef SERIALISTLOOPER_INPUT_HANDLER_H
#define SERIALISTLOOPER_INPUT_HANDLER_H

#include <juce_gui_basics/juce_gui_basics.h>
#include "generative_component.h"
#include "io/key_state.h"
#include "drag_and_drop_LEGACY.h"
#include "input_mode.h"
#include "input_condition.h"
#include "input_mode_map.h"
#include "interaction/drag_and_drop/drag_and_drop.h"

namespace serialist {

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
                 , Vec<std::reference_wrapper<Listener>> listeners
                 , GlobalDragAndDropContainer* global_dnd_container
                 , std::string identifier = "")
            : m_parent(parent)
              , m_mouse_source_component(mouse_source_component)
              , m_modes(std::move(modes))
              , m_listeners(std::move(listeners))
              , m_global_dnd_container(global_dnd_container)
              , m_drag_controller(global_dnd_container
                                  ? std::make_unique<DragController>(&mouse_source_component, *global_dnd_container)
                                  : nullptr)
              , m_identifier(std::move(identifier)) {

        if (m_parent) {
            m_parent->add_child_handler(*this);
        }
        m_mouse_source_component.addMouseListener(this, false);

        if (m_global_dnd_container) {
            m_global_dnd_container->add_listener(*this);
        }

        GlobalKeyState::add_listener(*this);

        if (update_active_mode()) {
            notify_listeners();
        }

    }


    ~InputHandler() override {
        if (m_parent) {
            m_parent->remove_child_handler(*this);
        }

        m_mouse_source_component.removeMouseListener(this);

        if (m_global_dnd_container) {
            m_global_dnd_container->remove_listener(*this);
        }

        GlobalKeyState::remove_listener(*this);
    }


    InputHandler(const InputHandler&) = delete;

    InputHandler& operator=(const InputHandler&) = delete;

    InputHandler(InputHandler&&) noexcept = delete;

    InputHandler& operator=(InputHandler&&) noexcept = delete;


    void mouseEnter(const juce::MouseEvent& event) override {
        mouse_enter_internal(event);
    }



    void mouseExit(const juce::MouseEvent& event) override {
        mouse_exit_internal(event);
    }





    void mouseDown(const juce::MouseEvent& event) override {
        mouse_down_internal(event);
    }


    void mouseMove(const juce::MouseEvent& event) override {
        mouse_move_internal(event, false);
    }



    void mouseDrag(const juce::MouseEvent& event) override {
        mouse_drag_internal(event, false);
    }


    void mouseUp(const juce::MouseEvent& event) override {
        mouse_up_internal(event, false);
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
        //       so there's no need to check interception status, etc.
        std::cout << m_identifier << "::Drop enter\n";
        m_last_mouse_state.drop_enter();
        if (update_mouse_state(false)) {
            notify_listeners();
        }
    }


    void drop_exit(const DragInfo&) override {
        std::cout << m_identifier << "::Drop exit\n";
        m_last_mouse_state.drop_exit();
        if (update_mouse_state(false)) {
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
        if (update_mouse_state(false)) {
            notify_listeners();
        }
    }


    void external_dnd_action_ended(const DragInfo& source) override {
        assert(interested_in(source)); // Transitive from caller calling interested_in

        m_last_mouse_state.external_drag_end();
        std::cout << m_identifier << "::External drag end\n";
        if (update_mouse_state(false)) {
            notify_listeners();
        }
    }


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


    GlobalDragAndDropContainer* get_global_dnd_container() const {
        return m_global_dnd_container;
    }

private:

    /**
    * @return true if mouse is over the component or any of its child components
    */
    bool mouse_is_over() const {
        return m_mouse_source_component.isMouseOver(true);
    }


    bool mouse_event_targets_this(bool already_consumed) const {
        return m_last_mouse_state.should_receive_interceptable_event() && !already_consumed;
    }


    void mouse_enter_from_child(const juce::MouseEvent& source_event) {
        mouse_enter_internal(source_event.getEventRelativeTo(&m_mouse_source_component));
    }


    void mouse_enter_internal(const juce::MouseEvent& event) {
        std::cout << "¤¤¤" << m_identifier << "::Event - MouseEnter\n";

        m_last_mouse_state.mouse_enter(event);
        update_intercepting();

        if (update_mouse_state(false)) {
            notify_listeners();
        }

        if (m_parent)
            m_parent->mouse_enter_from_child(event);
    }


    void mouse_exit_from_child(const juce::MouseEvent& source_event) {
        mouse_exit_internal(source_event.getEventRelativeTo(&m_mouse_source_component));
    }


    void mouse_exit_internal(const juce::MouseEvent& event) {
        if (!mouse_is_over()) {
            std::cout << "¤¤¤" << m_identifier << "::Event - MouseExit\n";
            // mouse exited this (either directly or from a child)
            m_last_mouse_state.mouse_exit();
            update_intercepting();

            if (update_mouse_state(false)) {
                notify_listeners();
            }

            if (m_parent)
                m_parent->mouse_exit_from_child(event);
        }

        // There are two cases here where neither this nor parent are notified:
        //  - if exiting this into a child (from this' pov, it's still over)
        //  - if a grandchild internally exits into a child (from this' pov, it's still over)
    }


    void mouse_down_from_child(const juce::MouseEvent& source_event) {
        mouse_down_internal(source_event.getEventRelativeTo(&m_mouse_source_component));
    }


    void mouse_down_internal(const juce::MouseEvent& event) {
        // Note: children will not pass down already consumed mouseDown events,
        //       hence if a mouseDown event is received, it is valid for this component
        std::cout << "¤¤¤" << m_identifier << "::Event - MouseDown\n";

        if (mouse_event_targets_this(false)) {
            // only register mouse_down if targeting this
            auto hide = m_active_mode && m_active_mode->get_drag_behaviour() == DragBehaviour::hide_and_restore;
            m_last_mouse_state.mouse_down(event, hide);
            if (update_mouse_state(false)) {
                notify_listeners();
            }

        } else {
            if (m_parent)
                m_parent->mouse_down_from_child(event);
        }
    }


    void mouse_move_from_child(const juce::MouseEvent& source_event, bool already_consumed) {
        mouse_move_internal(source_event.getEventRelativeTo(&m_mouse_source_component), already_consumed);
    }


    void mouse_move_internal(const juce::MouseEvent& event, bool already_consumed) {
        m_last_mouse_state.mouse_move(event);

        if (mouse_event_targets_this(already_consumed)) {
            if (update_mouse_state(true)) {
                notify_listeners();
            }
            already_consumed = true;
        }

        if (m_parent)
            m_parent->mouse_move_from_child(event, already_consumed);
    }


    void mouse_drag_from_child(const juce::MouseEvent& source_event, bool already_consumed) {
        mouse_drag_internal(source_event.getEventRelativeTo(&m_mouse_source_component), already_consumed);
    }


    void mouse_drag_internal(const juce::MouseEvent& event, bool already_consumed) {
        if (mouse_event_targets_this(already_consumed)) {
//            std::cout << "¤¤¤" << m_identifier << "::Event - MouseDrag\n";
            process_mouse_drag(event);
            already_consumed = true;
        }

        if (m_parent)
            m_parent->mouse_drag_from_child(event, already_consumed);
    }


    void process_mouse_drag(const juce::MouseEvent& event) {
        // TODO: We probably need to implement a MouseClickEvent and determine whether a mouseUp + mouseDown
        //      was a click or a drag (to avoid minimal accidental movements during click turning into drags)

        // an ongoing drag has previously been cancelled. No further actions until mouse button is released
        if (m_drag_cancelled) {
            return;
        }

        // ongoing drag edit within this component
        if (m_last_mouse_state.is_drag_editing) {
            m_last_mouse_state.mouse_drag_edit(event);
            if (update_mouse_state(true)) {
                notify_listeners();
            }
            return;
        }


        // ongoing drag and drop from this component
        if (m_last_mouse_state.is_dragging_from) {
            if (update_mouse_state(true)) {
                notify_listeners();
            }
            return;
        }

        // The mouse_down event was not registered by this component, meaning that the state changed in the middle
        // of a drag, and we should therefore not register any new drag operations for this component
        if (!m_last_mouse_state.is_down) {
            return;
        }

        // new drag edit in this component without any active state or with default drag edit behaviour
        if (m_active_mode && (m_active_mode->get_drag_behaviour() == DragBehaviour::drag_edit
                              || m_active_mode->get_drag_behaviour() == DragBehaviour::hide_and_restore)) {
            m_last_mouse_state.mouse_drag_edit(event);
            if (update_mouse_state(false)) {
                notify_listeners();
            }
            return;
        }

        // new drag and drop from this component
        if (m_active_mode && m_active_mode->get_drag_behaviour() == DragBehaviour::drag_and_drop) {
            start_drag_from(event);
            if (update_mouse_state(false)) {
                notify_listeners();
            }
            return;
        }

        // DragBehaviour::HideAndRestoreCursor is handled on mouseDown and mouseUp, not mouseDrag
    }


    void mouse_up_from_child(const juce::MouseEvent& event, bool already_consumed) {
        mouse_up_internal(event.getEventRelativeTo(&m_mouse_source_component), already_consumed);
    }


    void mouse_up_internal(const juce::MouseEvent& event, bool already_consumed) {
        if (mouse_event_targets_this(already_consumed)) {
            std::cout << "¤¤¤" << m_identifier << "::Event - MouseUp\n";
            if (m_last_mouse_state.is_dragging_from) {
                finalize_drag_from();
            } else {
                auto restore = m_active_mode && m_active_mode->get_drag_behaviour() == DragBehaviour::hide_and_restore;
                m_last_mouse_state.mouse_up(event, restore);
                if (update_mouse_state(false)) {
                    notify_listeners();
                }
            }
            already_consumed = true;
        } else {
            m_last_mouse_state.mouse_up(event); // TODO: not sure how to handle restore here
            // TODO: Not sure if we should notify anything here
        }

        re_enable_drag_after_cancel();

        if (m_parent)
            m_parent->mouse_up_from_child(event, already_consumed);
    }


    void start_drag_from(const juce::MouseEvent& mouse_event) {
        assert(m_drag_controller);
        assert(m_active_mode);
        assert(m_active_mode->get_drag_behaviour() == DragBehaviour::drag_and_drop);

        m_drag_controller->start_drag(m_active_mode->get_drag_info(), mouse_event, m_active_mode->get_drag_image());
        m_last_mouse_state.drag_start();

        if (update_mouse_state(false)) {
            notify_listeners();
        }
    }


    void finalize_drag_from() {
        assert(m_drag_controller);
        assert(m_active_mode);
        assert(m_active_mode->get_drag_behaviour() == DragBehaviour::drag_and_drop);

        m_drag_controller->finalize_drag();
        m_last_mouse_state.drag_end();

        if (update_mouse_state(false)) {
            notify_listeners();
        }
    }


    void cancel_all_drag_actions() {
//        std::cout << m_identifier << "::CANCELLING ALL DND\n";
        if (m_drag_controller)
            m_drag_controller->cancel_drag();

        if (m_last_mouse_state.is_dragging())
            m_drag_cancelled = true;

        m_last_mouse_state.reset_drag_state();
    }


    void re_enable_drag_after_cancel() {
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
            update_intercepting();

            if (m_active_mode) {
                m_active_mode->mode_activated();
                update_mouse_state(false);
//                std::cout << m_identifier << "::new mode\n";
            } else {
                m_last_state = NO_STATE;
//                std::cout << m_identifier << "::no mode\n";
            }

            return true;
        }

        return false;
    }


    /**
     * @return true if the state of the active mode has changed
     */
    bool update_mouse_state(bool only_position_changed) {
        if (m_active_mode) {
//            std::cout << m_identifier << "::updating mouse state / state\n";
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
            update_intercepting();
            std::cout << m_identifier << "::new state: " << m_last_state << std::endl;
            return true;
        }
        return false;
    }


    void update_intercepting() {
        bool should_intercept = m_active_mode &&
                                m_active_mode->intercept_mouse() &&
                                m_last_mouse_state.is_over_component();
//        std::cout << m_identifier << "::intercepting (" << m_last_mouse_state.is_intercepting << " -> " << should_intercept << ")" << std::endl;
        if (should_intercept != m_last_mouse_state.is_intercepting) {
            std::cout << m_identifier << "::new _INTERCEPTING_ state: " << should_intercept << std::endl;
            m_last_mouse_state.is_intercepting = should_intercept;

            if (m_parent) {
                m_parent->on_child_intercepting_change(m_last_mouse_state.is_intercepting
                                                       || m_last_mouse_state.is_intercepted);
            }

            for (const auto& child: m_child_handlers) {
                child.get().on_parent_intercepting_change(m_last_mouse_state.parent_wants_to_intercept
                                                          || m_last_mouse_state.is_intercepting);
            }
        }

        // Note: There's no need to notify any listener on `intercepting` change,
        //       they are generally only interested in `intercepted`.
    }


    void on_child_intercepting_change(bool is_intercepting) {
        bool was_intercepted = m_last_mouse_state.is_intercepted;
        m_last_mouse_state.is_intercepted = is_intercepting;

        if (was_intercepted != m_last_mouse_state.is_intercepted) {
            std::cout << m_identifier << "::new INTERCEPTED state: " << m_last_mouse_state.is_intercepted << std::endl;
            if (update_mouse_state(false)) {
                notify_listeners();
            }

            if (m_parent)
                m_parent->on_child_intercepting_change(
                        m_last_mouse_state.is_intercepting || m_last_mouse_state.is_intercepted);
        }
    }


    void on_parent_intercepting_change(bool wants_to_intercept) {
        bool parent_wanted_to_intercept = m_last_mouse_state.parent_wants_to_intercept;
        m_last_mouse_state.parent_wants_to_intercept = wants_to_intercept;

        if (parent_wanted_to_intercept != m_last_mouse_state.parent_wants_to_intercept) {
            std::cout << m_identifier << "::new PARENT_WANTS_TO_INTERCEPT state: "
                      << m_last_mouse_state.parent_wants_to_intercept << std::endl;

            if (!m_last_mouse_state.is_active_over_component()) {
                cancel_all_drag_actions();
            }

            if (update_mouse_state(false)) {
                notify_listeners();
            }

            for (const auto& child: m_child_handlers) {
                child.get().on_parent_intercepting_change(m_last_mouse_state.parent_wants_to_intercept
                                                          || m_last_mouse_state.is_intercepting);
            }
        }
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
    Vec<std::reference_wrapper<InputHandler> > m_child_handlers;

    juce::Component& m_mouse_source_component;

    InputModeMap m_modes;

    Vec<std::reference_wrapper<Listener>> m_listeners;

    GlobalDragAndDropContainer* m_global_dnd_container;
    std::unique_ptr<DragController> m_drag_controller;

    InputMode* m_active_mode = nullptr;
    int m_last_state = NO_STATE;


    MouseState m_last_mouse_state;

    std::string m_identifier;

    bool m_drag_cancelled = false;
};


} // namespace serialist

#endif //SERIALISTLOOPER_INPUT_HANDLER_H
