

#ifndef GUIUTILS_MOUSE_STATE_H
#define GUIUTILS_MOUSE_STATE_H

#include <optional>
#include <juce_gui_extra/juce_gui_extra.h>
#include "core/collections/circular_buffer.h"

struct MouseState {
    void mouse_enter(const juce::MouseEvent& event) {
        position = event.getPosition();
    }

    /**
     * should be called when the mouse enters a child (either directly or from parent)
     */
    void mouse_child_enter(const juce::MouseEvent& event) {
        mouse_enter(event);
        is_over_child = true;
    }


    void mouse_move(const juce::MouseEvent& event) {
        position = event.getPosition();
    }


    void mouse_down(const juce::MouseEvent& event, bool hide_mouse = false) {
        if (hide_mouse)
            event.source.enableUnboundedMouseMovement(true);

        is_down = true;
        position = event.getPosition();
        m_previous_drag_position = event.position;
    }


    void mouse_up(const juce::MouseEvent& event, bool restore = false) {
        is_down = false;
        is_dragging = false;
        if (restore) {
            std::cout << "restoring\n";
            restore_mouse_to_mouse_down_position(event);
        }

        position = event.getPosition();
        m_previous_drag_position = std::nullopt;
        drag_deplacement = std::nullopt;
    }


    /**
     * @return true if this is the start of a drag
     */
    bool mouse_drag(const juce::MouseEvent& event) {
        bool was_dragging = is_dragging;
        is_dragging = true;
        drag_deplacement = event.getOffsetFromDragStart();
        update_drag_velocity(event);
        position = event.getPosition();
        return !was_dragging;
    }

    /**
     * should be called when the mouse exits a child without exiting the parent component
     */
    void mouse_child_exit() {
        is_over_child = false;
    }

    void mouse_exit() {
        mouse_child_exit();
        is_down = false;
        is_dragging = false;
        position = std::nullopt;
        m_previous_drag_position = std::nullopt;
    }

    bool is_over_component() const {
        return position.has_value();
    }

    bool is_directly_over_component() const {
        return is_over_component() && !is_over_child;
    }


    std::optional<juce::Point<int>> position = std::nullopt;
    bool is_over_child = false;
    bool is_down = false;
    bool is_dragging = false;
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
