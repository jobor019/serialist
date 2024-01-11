
#ifndef SERIALISTLOOPER_INPUT_HANDLER_H
#define SERIALISTLOOPER_INPUT_HANDLER_H

#include <juce_gui_basics/juce_gui_basics.h>
#include "generative_component.h"
#include "key_state.h"
#include "drag_and_drop.h"
#include "input_mode.h"
#include "input_condition.h"
#include "input_mode_map.h"


class InputHandler
        : public GlobalKeyState::Listener
          , public juce::MouseListener
          , public DndTarget {
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
                 , Vec<std::reference_wrapper<Listener>> listeners)
            : m_parent(parent)
              , m_mouse_source_component(mouse_source_component)
              , m_modes(std::move(modes))
              , m_listeners(std::move(listeners)) {
        if (m_parent) {
            m_parent->add_child_handler(*this);
        }
        m_mouse_source_component.addMouseListener(this, true);
        GlobalKeyState::add_listener(*this);

        if (update_active_mode()) {
            notify_listeners();
        }
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
        bool is_directly_over = mouse_is_directly_over();

        // TODO: Not sure if the position of the mouse will be correct if passed from a child component
        // entering this component from outside
        if (!m_last_mouse_state.is_over_component()) {
            // entering registered child directly from outside
            if (!is_directly_over) {
                m_last_mouse_state.mouse_child_enter(event);
                if (update_mouse_position(false)) {
                    notify_listeners();
                }

                // entering this component (or a non-registered child) from outside
            } else {
                m_last_mouse_state.mouse_enter(event);
                if (update_mouse_position(false)) {
                    notify_listeners();
                }
            }

            // entering registered child from inside this component
        } else if (m_last_mouse_state.is_directly_over_component() && !is_directly_over) {
            m_last_mouse_state.mouse_child_enter(event);
            if (update_mouse_position(false)) {
                notify_listeners();
            }

        }
        // entering a non-registered component from a registered child does not need to be registered,
        // as this will first result in a mouseExit call


    }


    void mouseMove(const juce::MouseEvent& event) override {
        m_last_mouse_state.mouse_move(event);
        if (update_mouse_position(true)) {
            notify_listeners();
        }
    }


    void mouseExit(const juce::MouseEvent&) override {
        bool is_over = mouse_is_over();

        // exit this component (from this component or any registered/unregistered child)
        if (!is_over) {
            m_last_mouse_state.mouse_exit();
            if (update_mouse_position(false)) {
                notify_listeners();
            }
        } else if (!m_last_mouse_state.is_directly_over_component() && mouse_is_directly_over()) {
            // exit a registered child into this component
            m_last_mouse_state.mouse_child_exit();
            if (update_mouse_position(false)) {
                notify_listeners();
            }
        }
    }


    void mouseDown(const juce::MouseEvent& event) override {
        auto hide = m_active_mode && m_active_mode->get_drag_behaviour().is<DragBehaviour::HideAndRestoreCursor>();
        m_last_mouse_state.mouse_down(event, hide);
        if (update_mouse_position(false)) {
            notify_listeners();
        }
    }


    void mouseDrag(const juce::MouseEvent& event) override {
        // TODO: We probably need to implement a MouseClickEvent and determine whether a mouseUp + mouseDown
        //      was a click or a drag (to avoid minimal accidental movements during click turning into drags)

        // ongoing drag edit within this component
        if (m_last_mouse_state.is_drag_editing) {
            m_last_mouse_state.mouse_drag(event);
            if (update_mouse_position(true)) {
                notify_listeners();
            }
            return;
        }

        // ongoing drag and drop from this component
        if (m_dnd_state.is_dragging_from) {
            if (update_mouse_position(true)) {
                notify_listeners();
            }
            return;
        }

        // new drag edit in this component without any active state or with default drag behaviour
        if (!m_active_mode || m_active_mode->get_drag_behaviour().is<DragBehaviour::DefaultDrag>()) {
            m_last_mouse_state.mouse_drag(event);
            if (update_mouse_position(false)) {
                notify_listeners();
            }
            return;
        }


        // TODO: mouse_is_directly_over isn't really relevant here: what we need to know is
        //       if a child is in an active mode that listens to any type of drag action
        auto dnd = m_active_mode->get_drag_behaviour();
        if (dnd.is<DragBehaviour::DragAndDropFrom>() && mouse_is_directly_over()) {
            start_drag_from(dnd.as<DragBehaviour::DragAndDropFrom>());
            return;
        }

        // DragBehaviour::HideAndRestoreCursor is handled on mouseDown and mouseUp, not mouseDrag
    }


    void mouseUp(const juce::MouseEvent& event) override {
        if (m_dnd_state.is_dragging_from) {
            end_drag_from();
        } else {
            auto restore = m_active_mode &&
                           m_active_mode->get_drag_behaviour().is<DragBehaviour::HideAndRestoreCursor>();
            m_last_mouse_state.mouse_up(event, restore);
            if (update_mouse_position(false)) {
                notify_listeners();
            }
        }
        // Handle HideAndRestoreCursor case here
        // Also need to handle END of drag and drop actions here. use `end_drag_from()`
    }


    bool interested_in(const DndTarget::Details& source) override {
        return m_active_mode && m_active_mode->supports_drag_to(source);
    }


    void start_drag_from(const DragBehaviour::DragAndDropFrom& dnd_config) {
        // TODO: Will probably need a more elaborate solution than findParentDragContainer to
        //       differentiate different types of drag depending on mode (utilize parent InputHandler!).
        auto* parent = juce::DragAndDropContainer::findParentDragContainerFor(&m_mouse_source_component);

        if (parent && !parent->isDragAndDropActive()) {
            parent->startDragging("", &m_mouse_source_component, dnd_config.cursor_image.value_or(juce::ScaledImage()));
            m_dnd_state.is_dragging_from = true;
            if (update_mouse_position(false)) {
                notify_listeners();
            }
        }
    }


    void end_drag_from() {
        m_dnd_state.is_dragging_from = false;
        if (update_mouse_position(false)) {
            notify_listeners();
        }
    }


    void drag_enter(const DndTarget::Details&) override {
        m_dnd_state.is_dragging_to = true;
        if (update_mouse_position(false)) {
            notify_listeners();
        }
    }


    void drag_exit(const DndTarget::Details&) override {
        m_dnd_state.is_dragging_to = false;
        if (update_mouse_position(false)) {
            notify_listeners();
        }
    }


    void drag_move(const DndTarget::Details&) override {
        if (update_mouse_position(true)) {
            notify_listeners();
        }
    }


    void drop(const juce::DragAndDropTarget::SourceDetails& source) override {
        if (m_active_mode) {
            m_active_mode->input_event_registered(std::make_unique<DragDropped>(source));
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


private:

    /**
     * @return true if the active mode has changed
     */
    bool update_active_mode() {
        if (auto* active_mode = m_modes.get_active(); active_mode != m_active_mode) {

            if (m_active_mode) {
                m_active_mode->reset();
            }

            m_active_mode = active_mode;
            if (m_active_mode) {
                update_mouse_position(false);
            } else {
                m_last_state = NO_STATE;
            }

            return true;
        }

        return false;
    }


    /**
     * @return true if the state of the active mode has changed
     */
    bool update_mouse_position(bool only_internal_position_changed) {
        if (m_active_mode) {
            if (only_internal_position_changed) {
                return update_state(m_active_mode->mouse_position_changed(m_last_mouse_state, m_dnd_state));
            } else {
                return update_state(m_active_mode->mouse_state_changed(m_last_mouse_state, m_dnd_state));
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
            std::cout << "new state: " << m_last_state << std::endl;
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

    InputMode* m_active_mode = nullptr;
    int m_last_state = NO_STATE;

    Vec<std::reference_wrapper<Listener>> m_listeners;

    MouseState m_last_mouse_state;
    DragAndDropState m_dnd_state;
};


#endif //SERIALISTLOOPER_INPUT_HANDLER_H
