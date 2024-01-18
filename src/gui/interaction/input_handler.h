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
                 , Vec<std::reference_wrapper<Listener> > listeners
                 , std::string identifier = "")
        : m_parent(parent)
          , m_mouse_source_component(mouse_source_component)
          , m_modes(std::move(modes))
          , m_global_dnd_container(global_dnd_container)
          , m_drag_controller(&mouse_source_component, global_dnd_container)
          , m_listeners(std::move(listeners))
          , m_identifier(std::move(identifier)) {
        if (m_parent) {
            m_parent->add_child_handler(*this);
        }
        m_mouse_source_component.addMouseListener(this, false);
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
    //    bool mouse_is_directly_over() const {
    //        return mouse_is_over() && !mouse_is_over_child();
    //    }


    bool this_wants_to_intercept_mouse() const {
        return m_active_mode && m_active_mode->intercept_mouse();
    }


    bool parent_wants_to_intercept_mouse() const {
        return m_parent && m_parent->wants_to_intercept_mouse();
    }


    bool wants_to_intercept_mouse() const {
        return this_wants_to_intercept_mouse() || parent_wants_to_intercept_mouse();
    }


    //    /**
    //     * @return true if mouse is over a component that has a registered child StateHandler
    //     */
    //    bool mouse_is_over_child() const {
    //        return m_child_handlers.any([](const auto& handler) {
    //            return handler.get().mouse_is_over();
    //        });
    //    }


    //    juce::Component* component_under_mouse() const {
    //        if (!mouse_is_over()) {
    //            return nullptr;
    //        }
    //
    //        for (const auto& handler: m_child_handlers) {
    //            if (handler.get().mouse_is_over()) {
    //                return &handler.get().m_mouse_source_component;
    //            }
    //        }
    //
    //        return &m_mouse_source_component;
    //    }


    bool mouse_event_targets_this(bool consumed_by_child) const {
        // priority/interception order:
        //  1. child (consumed_by_child)
        //  2. this if intercepts_mouse
        //  3. parent if intercepts_mouse
        //  4. this if no interception occurred above
        return !consumed_by_child && (this_wants_to_intercept_mouse() || !parent_wants_to_intercept_mouse());
    }


    void mouseEnter(const juce::MouseEvent& event) override {
        mouse_enter_internal(event, false);
    }


    void mouse_enter_from_child(const juce::MouseEvent& source_event, bool already_consumed) {
        mouse_enter_internal(source_event.getEventRelativeTo(&m_mouse_source_component), already_consumed);
    }


    void mouse_enter_internal(const juce::MouseEvent& event, bool already_consumed) {
        std::cout << "¤¤¤" << m_identifier << "::Event - MouseEnter\n";

        m_last_mouse_state.mouse_enter(event);

        if (mouse_event_targets_this(already_consumed)) {
            if (update_mouse_state(false)) {
                notify_listeners();
            }
            already_consumed = true;
        }

        if (m_parent)
            m_parent->mouse_enter_from_child(event, already_consumed);
    }


    void mouseMove(const juce::MouseEvent& event) override {
        mouse_move_internal(event, false);
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


    void mouseExit(const juce::MouseEvent& event) override {
        mouse_exit_internal(event);
    }


    void mouse_exit_from_child(const juce::MouseEvent& source_event) {
        mouse_exit_internal(source_event.getEventRelativeTo(&m_mouse_source_component));
    }


    void mouse_exit_internal(const juce::MouseEvent& event) {
        std::cout << "¤¤¤" << m_identifier << "::Event - MouseExit\n";
        if (!mouse_is_over()) {
            // mouse exited this (either directly or from a child)
            m_last_mouse_state.mouse_exit();
            if (update_mouse_state(false)) {
                notify_listeners();
            }

            if (m_parent)
                m_parent->mouse_exit_from_child(event);
        } else {
            // TODO Remove
            // mouse exited a direct child but is still over this
            // m_last_mouse_state.mouse_child_exit();
            // if (update_mouse_state(false)) {
            //     notify_listeners();
            // }

            // parent isn't notified here as internally exiting a grandchild into a child is irrelevant to the parent
        }
    }


    void mouseDown(const juce::MouseEvent& event) override {
        mouse_down_internal(event, false);
    }


    void mouse_down_from_child(const juce::MouseEvent& source_event, bool already_consumed) {
        mouse_down_internal(source_event.getEventRelativeTo(&m_mouse_source_component), already_consumed);
    }


    void mouse_down_internal(const juce::MouseEvent& event, bool already_consumed) {
        std::cout << "¤¤¤" << m_identifier << "::Event - MouseDown\n";
        if (mouse_event_targets_this(already_consumed)) {
            // only register mouse_down if targeting this
            auto hide = m_active_mode && m_active_mode->get_drag_behaviour() == DragBehaviour::hide_and_restore;
            m_last_mouse_state.mouse_down(event, hide);
            if (update_mouse_state(false)) {
                notify_listeners();
            }
            already_consumed = true;
        }

        if (m_parent)
            m_parent->mouse_down_from_child(event, already_consumed);
    }


    void mouse_drag_internal(const juce::MouseEvent& event, bool already_consumed) {
        std::cout << "¤¤¤" << m_identifier << "::Event - MouseDrag\n";
        if (mouse_event_targets_this(already_consumed)) {
            process_mouse_drag(event);
            already_consumed = true;
        }

        if (m_parent)
            m_parent->mouse_drag_from_child(event, already_consumed);
    }


    void mouse_drag_from_child(const juce::MouseEvent& source_event, bool already_consumed) {
        mouse_drag_internal(source_event.getEventRelativeTo(&m_mouse_source_component), already_consumed);
    }


    void mouseDrag(const juce::MouseEvent& event) override {
        mouse_drag_internal(event, false);
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

        // new drag edit in this component without any active state or with default drag edit behaviour
        if (m_active_mode && m_active_mode->get_drag_behaviour() == DragBehaviour::drag_edit) {
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


    void mouseUp(const juce::MouseEvent& event) override {
        mouse_up_internal(event, false);
    }


    void mouse_up_from_child(const juce::MouseEvent& event, bool already_consumed) {
        mouse_up_internal(event.getEventRelativeTo(&m_mouse_source_component), already_consumed);
    }


    void mouse_up_internal(const juce::MouseEvent& event, bool already_consumed) {
        // TODO: This solution is definitely not correct, but will need clear test cases to find the correct solution
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


    GlobalDragAndDropContainer& get_global_dnd_container() const {
        return m_global_dnd_container;
    }

private:
    void start_drag_from(const juce::MouseEvent& mouse_event) {
        assert(m_active_mode);
        assert(m_active_mode->get_drag_behaviour() == DragBehaviour::drag_and_drop);

        m_drag_controller.start_drag(m_active_mode->get_drag_info(), mouse_event, m_active_mode->get_drag_image());
        m_last_mouse_state.drag_start();

        if (update_mouse_state(false)) {
            notify_listeners();
        }
    }


    void finalize_drag_from() {
        assert(m_active_mode);
        assert(m_active_mode->get_drag_behaviour() == DragBehaviour::drag_and_drop);

        m_drag_controller.finalize_drag();
        m_last_mouse_state.drag_end();

        if (update_mouse_state(false)) {
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

            if (m_active_mode) {
                m_active_mode->mode_activated();
                update_mouse_state(false);
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
    bool update_mouse_state(bool only_position_changed) {
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
        std::cout << m_identifier << "update intercepting" << std::endl;

        if (should_intercept != m_last_mouse_state.is_intercepting) {
            std::cout << m_identifier << "::new intercepting state: " << should_intercept << std::endl;
            m_last_mouse_state.is_intercepting = should_intercept;

            if (m_parent) {
                m_parent->on_child_intercepting_change(m_last_mouse_state.is_intercepting);
            }
        }
        // Note: There's no need to notify any listener on `intercepting` change,
        //       they are generally only interested in `intercepted`.
    }


    void on_child_intercepting_change(bool is_intercepting) {
        bool was_intercepted = m_last_mouse_state.is_intercepted;
        m_last_mouse_state.is_intercepted = is_intercepting;

        std::cout << m_identifier << "update intercepted" << std::endl;

        if (was_intercepted != m_last_mouse_state.is_intercepted) {
            std::cout << m_identifier << "::new INTERCEPTED state: " << m_last_mouse_state.is_intercepted << std::endl;
            if (update_mouse_state(false)) {
                notify_listeners();
            }

            if (m_parent)
                m_parent->on_child_intercepting_change(is_intercepting);
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

    GlobalDragAndDropContainer& m_global_dnd_container;
    DragController m_drag_controller;

    InputMode* m_active_mode = nullptr;
    int m_last_state = NO_STATE;

    Vec<std::reference_wrapper<Listener> > m_listeners;

    MouseState m_last_mouse_state;

    std::string m_identifier;

    bool m_drag_cancelled = false;
};


#endif //SERIALISTLOOPER_INPUT_HANDLER_H
