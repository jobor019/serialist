

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


// ==============================================================================================

struct MouseState {
    void mouse_enter(const juce::MouseEvent& event) {
        position = event.getPosition();
    }


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
        update_drag_velocity(event);
        position = event.getPosition();
        return !was_dragging;
    }


    void mouse_exit() {
        reset_drag_state();
        is_down = false;
        position = std::nullopt;
    }


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
        return position.has_value() && should_receive_interceptable_event();
    }


    bool is_dragging() const {
        return is_drag_editing || is_drag_and_dropping();
    }

    bool is_drag_and_dropping() const {
        return is_dragging_from || is_dragging_to;
    }

    bool should_receive_interceptable_event() const {
        // priority/interception order:
        //  1. child
        //  2. this if this intercepts mouse
        //  3. parent if parent intercepts mouse
        //  4. this if no interception occurred above
        return !is_intercepted && (!parent_wants_to_intercept || is_intercepting);
    }

    bool parent_should_receive_interceptable_event() const {
        return parent_wants_to_intercept && !is_intercepting && !is_intercepted;
    }

    bool child_should_receive_interceptable_event() const {
        return is_intercepted;
    }

    std::optional<juce::Point<int>> position = std::nullopt;


    bool is_intercepting = false;
    bool is_intercepted = false;
    bool parent_wants_to_intercept = false;

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
