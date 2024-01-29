
#ifndef SERIALISTLOOPER_INPUT_MODE_H
#define SERIALISTLOOPER_INPUT_MODE_H

#include <optional>
#include "mouse_state.h"
#include "input_events.h"
#include "interaction/drag_and_drop/drag_and_drop.h"


class InputMode {
public:
    enum class DragImageType {
        none = 0
        , default_snapshot = 1
        , custom_fixed = 2
        , custom_dynamic = 3
    };

    InputMode() = default;

    virtual ~InputMode() = default;

    InputMode(const InputMode&) = delete;

    InputMode& operator=(const InputMode&) = delete;

    InputMode(InputMode&&) noexcept = default;

    InputMode& operator=(InputMode&&) noexcept = default;

    /**
     * @return true if mouse should be intercepted when clicking (non-intercepting) child components
     */
    virtual bool intercept_mouse() = 0;

    virtual DragBehaviour get_drag_behaviour() { return DragBehaviour::drag_edit; }

    virtual std::unique_ptr<DragInfo> get_drag_info() {
        // should be overridden if mode supports drag and drop
        assert(get_drag_behaviour() != DragBehaviour::drag_and_drop);
        return nullptr;
    }

    virtual bool supports_drop_from(const DragInfo& source) const { (void) source; return false; }

    /**
     * @note: if has_drag_image is true but get_drag_image returns nullopt,
     *  it will just create a snapshot of entire mouse component
     */
    virtual DragImageType has_drag_image() const { return DragImageType::default_snapshot; }


    virtual std::optional<juce::ScaledImage> get_drag_image() const { return std::nullopt; }

    virtual std::optional<int> item_dropped(const DragInfo& source) {
        (void) source;
        return std::nullopt;
    }

    virtual std::optional<int> mode_activated() { return std::nullopt; }

    virtual std::optional<int> mode_deactivated() { return std::nullopt; }


    /**
     * @return current state (std::nullopt may be used as a shortcut for no state change)
     */
    virtual std::optional<int> mouse_state_changed(const MouseState& mouse_state) {
        (void) mouse_state;
        return std::nullopt;
    }

    virtual std::optional<int> mouse_position_changed(const MouseState& mouse_state) {
        (void) mouse_state;
        return std::nullopt;
    }

    /**
     * @return current state (std::nullopt may be used as a shortcut for no state change)
     */
    virtual std::optional<int> input_event_registered(std::unique_ptr<InputEvent> input_event) {
        (void) input_event;
        return std::nullopt;
    }


    bool operator==(const InputMode& other) const {
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
