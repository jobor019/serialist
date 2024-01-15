
#ifndef SERIALISTLOOPER_INPUT_MODE_H
#define SERIALISTLOOPER_INPUT_MODE_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <optional>
#include "mouse_state.h"
#include "drag_and_drop_LEGACY.h"
#include "input_events.h"



class InputMode {
public:
    InputMode() = default;
    virtual ~InputMode() = default;
    InputMode(const InputMode&) = delete;
    InputMode& operator=(const InputMode&) = delete;
    InputMode(InputMode&&)  noexcept = default;
    InputMode& operator=(InputMode&&)  noexcept = default;

    virtual DragBehaviour get_drag_behaviour() = 0;
    virtual bool supports_drag_to(const DndTarget::Details& source) const = 0;


    /**
     * @note: if has_drag_image is true but get_drag_image returns nullopt,
     *  it will just create a snapshot of entire mouse component
     */
    virtual bool has_drag_image() const { return false; }
    virtual std::optional<juce::ScaledImage> get_drag_image() const { return std::nullopt; }

    /**
     * @return current state (std::nullopt may be used as a shortcut for no state change)
     */
    virtual std::optional<int> mouse_state_changed(const MouseState& mouse_state
                                    , const DragAndDropState& dnd_state) = 0;

    virtual std::optional<int> mouse_position_changed(const MouseState& mouse_state
    , const DragAndDropState& dnd_state) = 0;

    /**
     * @return current state (std::nullopt may be used as a shortcut for no state change)
     */
    virtual std::optional<int> input_event_registered(std::unique_ptr<InputEvent> input_event) = 0;

    // TODO: Not sure if this is needed
    // virtual int get_state() const = 0;


    bool operator==(const InputMode& other) const {
        std::cout << "input mode (comparison)\n";
        return typeid(*this) == typeid(other);
    }

    /**
     * @note: called when the mode is no longer active. The mode is generally not destroyed when no longer in use,
     *  but rather reset to its original state when unused
     */
    virtual void reset() = 0;

    template<typename T>
    bool is() const {
        return typeid(T) == typeid(*this);
    }

};

#endif //SERIALISTLOOPER_INPUT_MODE_H
