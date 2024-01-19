

#ifndef GUIUTILS_MOUSE_STATE_H
#define GUIUTILS_MOUSE_STATE_H

#include <optional>
#include <juce_gui_extra/juce_gui_extra.h>
#include "core/collections/circular_buffer.h"

enum class DragBehaviour {
    not_supported = 0
    , drag_edit = 1
    , hide_and_restore = 2
    , drag_and_drop = 3
};

//class DragBehaviour {
//public:
//    struct DefaultDrag {};
//    struct HideAndRestoreCursor {};
//    struct DragAndDropFrom { std::optional<juce::ScaledImage> cursor_image = std::nullopt; };
//
//    using BehaviourType = std::variant<DefaultDrag, HideAndRestoreCursor, DragAndDropFrom>;
//
//    DragBehaviour() = default;
//
//
//    DragBehaviour(BehaviourType&& behaviour) : m_behaviour(std::move(behaviour)) {}
//
//
//    template<typename T>
//    bool is() const noexcept {
//        return std::holds_alternative<T>(m_behaviour);
//    }
//
//
//    template<typename T>
//    T as() const {
//        return std::get<T>(m_behaviour);
//    }
//
//
//private:
//    BehaviourType m_behaviour = DefaultDrag{};
//};


// ==============================================================================================

struct MouseState {
    void mouse_enter(const juce::MouseEvent& event) {
        position = event.getPosition();
    }


    // /**
    //  * should be called when the mouse enters a child (either directly or from parent)
    //  */
    // void mouse_child_enter(const juce::MouseEvent& event) {
    //     mouse_enter(event);
    //     is_over_child = true;
    // }


    void mouse_move(const juce::MouseEvent& event) {
        position = event.getPosition();
    }


    void mouse_down(const juce::MouseEvent& event, bool hide_mouse = false) {
        if (hide_mouse) {
            event.source.enableUnboundedMouseMovement(true);
            is_hidden = true;
        }

        is_down = true;
        position = event.getPosition();
        m_previous_drag_position = event.position;
    }


    void mouse_up(const juce::MouseEvent& event, bool restore = false) {
        is_down = false;

        if (is_hidden || restore) {
            restore_mouse_to_mouse_down_position(event);
            is_hidden = false;
        }

        is_drag_editing = false;

        position = event.getPosition();
        m_previous_drag_position = std::nullopt;
        drag_deplacement = std::nullopt;
    }


    /**
     * @return true if this is the start of a drag
     */
    bool mouse_drag_edit(const juce::MouseEvent& event) {
        bool was_dragging = is_drag_editing;
        is_drag_editing = true;
        drag_deplacement = event.getOffsetFromDragStart();
        std::cout << "drag deplacement: " << drag_deplacement->getY() << "\n";
        update_drag_velocity(event);
        position = event.getPosition();
        return !was_dragging;
    }


    // /**
    //  * should be called when the mouse exits a child without exiting the parent component
    //  */
    // void mouse_child_exit() {
    //     is_over_child = false;
    // }


    void mouse_exit() {
        reset_drag_state();
        is_down = false;
        position = std::nullopt;
    }


    // TODO: Not sure if these should even live in MouseState
    void drop_enter() {
        is_dragging_to = true;
    }


    void drop_exit() {
        is_dragging_to = false;
    }


    void drag_start() {
        is_dragging_from = true;
    }


    void drag_end() {
        is_dragging_from = false;
    }

    void external_drag_start() {
        has_external_ongoing_drag = true;
    }

    void external_drag_end() {
        has_external_ongoing_drag = false;
    }

    void reset_drag_state() {
        is_dragging_to = false;
        is_dragging_from = false;
        has_external_ongoing_drag = false;
        is_drag_editing = false;
        drag_deplacement = std::nullopt;
        m_previous_drag_position = std::nullopt;
    }


    bool is_over_component() const {
        return position.has_value();
    }

    bool is_active_over_component() const {
        std::cout << "positioni has value " << position.has_value() << ", is_intercepted " << is_intercepted << "\n";
        return position.has_value() && !is_intercepted;
    }

    // bool is_intercepted() const {
    //     return interception == Interception::intercepted;
    // }

    // void set_intercepting(bool intercepting) {
    //     is_intercepting = intercepting;
    //     if (intercepting)
    //         is_intercepted = false;
    // }


    // bool is_directly_over_component() const {
    //     return is_over_component() && !is_intercepted_by_child;
    // }

    bool is_dragging() const {
        return is_drag_editing || is_drag_and_dropping();
    }

    bool is_drag_and_dropping() const {
        return is_dragging_from || is_dragging_to;
    }


    std::optional<juce::Point<int>> position = std::nullopt;

    // enum class Interception {
    //     none
    //     , intercepting
    //     , intercepted
    // };
    //
    // Interception interception;

    bool is_intercepting = false;
    bool is_intercepted = false;

    bool is_down = false;
    bool is_drag_editing = false;

    bool is_dragging_from = false;
    bool is_dragging_to = false;
    bool has_external_ongoing_drag = false;

    bool is_hidden = false;

    float drag_velocity_x = 0.0f;
    float drag_velocity_y = 0.0f;
    std::optional<juce::Point<int>> drag_deplacement = std::nullopt;


private:

    void update_drag_velocity(const juce::MouseEvent& event) {
        auto pos = event.position;
        m_drag_position_buffer.push(*m_previous_drag_position - pos);
        auto buf = m_drag_position_buffer.get_buffer();


        drag_velocity_x = std::accumulate(
                buf.begin()
                , buf.end()
                , 0.0f
                , [](float sum, const auto& p) { return sum + static_cast<float>(p.getX()); }
        );

        drag_velocity_y = std::accumulate(
                buf.begin()
                , buf.end()
                , 0.0f
                , [](float sum, const auto& p) { return sum + static_cast<float>(p.getY()); }
        );

//        std::cout << "drag velocity (" << drag_velocity_x << ", " << drag_velocity_y << ")\n";

        m_previous_drag_position = pos;

    }


    void restore_mouse_to_mouse_down_position(const juce::MouseEvent& event) {
        event.source.enableUnboundedMouseMovement(false);
        juce::Desktop::getInstance()
                .getMainMouseSource()
                .setScreenPosition(event.getMouseDownScreenPosition().toFloat());
    }


    std::optional<juce::Point<float>> m_previous_drag_position;

    // TODO: Rewrite Buffer to handle dynamic setting of drag velocity
    Buffer<juce::Point<float>, 5> m_drag_position_buffer;
};


#endif //GUIUTILS_MOUSE_STATE_H
