
#ifndef SERIALISTLOOPER_INPUT_EVENTS_H
#define SERIALISTLOOPER_INPUT_EVENTS_H

#include <typeinfo>
#include "drag_and_drop_LEGACY.h"

// TODO: Not sure if this should be a variant or a dynamic polymorphic class
class InputEvent {
public:
    InputEvent() = default;
    virtual ~InputEvent() = default;
    InputEvent(const InputEvent&) = delete;
    InputEvent& operator=(const InputEvent&) = delete;
    InputEvent(InputEvent&&) noexcept = default;
    InputEvent& operator=(InputEvent&&) noexcept = default;


    template<typename T>
    bool is() const {
        std::cout << "input event\n";
        return typeid(T) == typeid(*this);
    }
};


// ==============================================================================================

class MouseClick : public InputEvent {};

class Trigger : public InputEvent {};

class DragDropped : public InputEvent {
public:
    explicit DragDropped(const juce::DragAndDropTarget::SourceDetails& source) : m_source(source) {}

    const juce::DragAndDropTarget::SourceDetails& get_source() const {
        return m_source;
    }

private:
    juce::DragAndDropTarget::SourceDetails m_source;
};

#endif //SERIALISTLOOPER_INPUT_EVENTS_H
