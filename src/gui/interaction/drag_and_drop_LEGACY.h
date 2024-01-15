
#ifndef SERIALISTLOOPER_DRAG_AND_DROP_LEGACY_H
#define SERIALISTLOOPER_DRAG_AND_DROP_LEGACY_H

#include <juce_gui_basics/juce_gui_basics.h>
#include "input_events.h"

struct DragAndDropState {
    bool is_active() const {
        return is_dragging_to || is_dragging_from;
    }

    bool is_dragging_to = false;
    bool is_dragging_from = false;
};


// ==============================================================================================

/**
 * Wrapper interface for juce::DragAndDropTarget (which requires implementation directly in a component
 */
class DndTarget {
public:
    using Details = juce::DragAndDropTarget::SourceDetails;

    DndTarget() = default;
    virtual ~DndTarget() = default;
    DndTarget(const DndTarget&) = delete;
    DndTarget& operator=(const DndTarget&) = delete;
    DndTarget(DndTarget&&)  noexcept = default;
    DndTarget& operator=(DndTarget&&)  noexcept = default;

    virtual bool interested_in(const Details& source) = 0;
    virtual void drag_enter(const Details& source) = 0;
    virtual void drag_exit(const Details& source) = 0;
    virtual void drag_move(const Details& source) = 0;
    virtual void drop(const Details& source) = 0;
};


// ==============================================================================================

#endif //SERIALISTLOOPER_DRAG_AND_DROP_LEGACY_H
