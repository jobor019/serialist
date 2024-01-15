
#ifndef SERIALISTLOOPER_DRAG_AND_DROP_H
#define SERIALISTLOOPER_DRAG_AND_DROP_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "core/collections/vec.h"

class DragAndDropState {
}; // TODO: Shouldn't this just be part of MouseState then?



class DragInfo {
public:
    DragInfo() = default;
    virtual ~DragInfo() = default;
    DragInfo(const DragInfo&) = delete;
    DragInfo& operator=(const DragInfo&) = delete;
    DragInfo(DragInfo&&) noexcept = default;
    DragInfo& operator=(DragInfo&&) noexcept = default;

    virtual bool equals(const DragInfo&) const = 0;
    virtual juce::ScaledImage get_image() const = 0;

};

class DummyDragInfo : public DragInfo {
public:
    bool equals(const DragInfo&) const override {
        return true;
    }
};

class DragImageComponent : public juce::Component {
public:
    explicit DragImageComponent(const DragInfo& info) : m_image(info.get_image()) {
        setInterceptsMouseClicks(false, false);
    }


    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::darksalmon); // TODO
    }


private:
    juce::ScaledImage m_image;

};


class DragController;


class DropListener {
public:
    DropListener() = default;
    virtual ~DropListener() = default;
    DropListener(const DropListener&) = delete;
    DropListener& operator=(const DropListener&) = delete;
    DropListener(DropListener&&) noexcept = default;
    DropListener& operator=(DropListener&&) noexcept = default;

    virtual bool interested_in(const DragInfo& source) = 0;

    virtual void drag_started(const DragInfo& source) = 0;
    virtual void drag_ended(const DragInfo& source) = 0;

    virtual void drag_enter(const DragInfo& source, const juce::MouseEvent& event) = 0;
    virtual void drag_exit(const DragInfo& source, const juce::MouseEvent& event) = 0;
    virtual void drag_move(const DragInfo& source, const juce::MouseEvent& event) = 0;

    virtual void on_drop(const DragInfo& source) = 0;

};


class DropAreaComponent {
public:
    DropAreaComponent() = default;
    virtual ~DropAreaComponent() = default;
    DropAreaComponent(const DropAreaComponent&) = delete;
    DropAreaComponent& operator=(const DropAreaComponent&) = delete;
    DropAreaComponent(DropAreaComponent&&) noexcept = delete;
    DropAreaComponent& operator=(DropAreaComponent&&) noexcept = delete;

    virtual DropListener& get_drop_listener() = 0;

private:
    JUCE_DECLARE_WEAK_REFERENCEABLE(DropAreaComponent)
};


class GlobalDragAndDropContainer : public juce::Component {
public:
    GlobalDragAndDropContainer() {
        addMouseListener(this, true);
    }


    void start_drag(const DragInfo& source, const juce::MouseEvent& event) {
        assert(m_ongoing_drag == nullptr);
        assign_drag(&source);

        if (auto target = find_target(event)) {
            m_dragged_over_component = target;
            target->get_drop_listener().drag_enter(source, event);
        }

//        if (!contains(source)) {
//            auto drag_image = std::make_unique<DragImageComponent>(source);
//            addAndMakeVisible(*drag_image);
//            m_ongoing_drags.append(std::move(drag_image));
//
//            notify_drag_start(source);
//
//            if (auto target = find_target()) {
//                m_dragged_over_component = target;
//                target->get_drop_listener().drag_enter(source);
//            }
//        }
    }


    void cancel_drag(const DragInfo& source) {
        if (is_dragging(source)) {
            if (has_valid_drop_target()) {
                m_dragged_over_component->get_drop_listener().drag_exit(source);
                m_dragged_over_component = nullptr;
            }

            assign_drag(nullptr);
        }

//        if (auto index = index_of(source)) {
//            if (!m_dragged_over_component.wasObjectDeleted()) {
//                m_dragged_over_component->get_drop_listener().drag_exit(source);
//                m_dragged_over_component = nullptr;
//            }
//
//            if (auto drag_image = m_ongoing_drags.pop_index(*index)) {
//                for (const auto& listener: m_listeners) {
//                    listener.get().drag_ended(source);
//                }
//            }
//        }
    }


    void finalize_drag(const DragInfo& source) {
        if (is_dragging(source)) {
            if (has_valid_drop_target()) {
                m_dragged_over_component->get_drop_listener().on_drop(source);
                m_dragged_over_component = nullptr;
            }

            assign_drag(nullptr);
        }


//        if (auto index = index_of(source)) {
//            if (!m_dragged_over_component.wasObjectDeleted()) {
//                m_dragged_over_component->get_drop_listener().on_drop(source);
//                m_dragged_over_component = nullptr;
//            }
//
//            if (auto drag_image = m_ongoing_drags.pop_index(*index)) {
//                notify_drag_end(source);
//            }
//        }
    }


    void update_drag_image(juce::ScaledImage& image, DragInfo& source) {
        (void) image;
        (void) source;
        // TODO
    }


    void add_listener(DropListener& listener) {
        m_listeners.append(listener);
    }


    void remove_listener(const DropListener& listener) {
        m_listeners.remove([&listener](const auto& l) { return &l.get() == &listener; });
    }


    bool is_dragging() const {
        return static_cast<bool>(m_ongoing_drag);
    }


    bool is_dragging(const DragInfo& source) const {
        return m_ongoing_drag == &source;
    }


    bool has_valid_drop_target() const {
        return !m_dragged_over_component.wasObjectDeleted();
    }


    void mouseDrag(const juce::MouseEvent& event) override {
        if (!is_dragging()) {
            return;
        }

        if (auto target = find_target(event)) {

            if (target == m_dragged_over_component.get()) {
                m_dragged_over_component->get_drop_listener().drag_move(*m_ongoing_drag, event);

            } else {
                // exit current component if existing
                if (has_valid_drop_target()) {
                    m_dragged_over_component->get_drop_listener().drag_exit(*m_ongoing_drag, event);
                }

                m_dragged_over_component = target;

                // if entering a new component, notify it
                if (has_valid_drop_target()) {
                    m_dragged_over_component->get_drop_listener().drag_enter(*m_ongoing_drag, event);
                }
            }


        }
    }

    // mouseUp etc. is NOT the responsibility of the GlobalDragAndDropContainer, it is handled by the DragController

    // For other listeners like the GUI visualizing ongoing connections between components
    DropAreaComponent* get_dragged_over_component() const {
        return m_dragged_over_component.get();
    }


private:
    DropAreaComponent* find_target(const juce::MouseEvent& event) {
        auto* component = getComponentAt(event.getEventRelativeTo(this).getPosition());

        while (component && component != this) {
            if (auto* drop_area_component = dynamic_cast<DropAreaComponent*>(component)) {
                if (drop_area_component->get_drop_listener().interested_in(*m_ongoing_drag)) {
                    return drop_area_component;
                }
            }

            component = component->getParentComponent();
        }

        return nullptr;
    }


    void assign_drag(const DragInfo* source) {
        if (source) {
            m_drag_image = std::make_unique<DragImageComponent>(*source);
            addAndMakeVisible(*m_drag_image);
            m_ongoing_drag = source;
            notify_drag_start(*source);

        } else {
            if (m_ongoing_drag)
                notify_drag_end(*m_ongoing_drag);

            m_ongoing_drag = nullptr;
            m_drag_image = nullptr;
        }
    }


    void notify_drag_end(const DragInfo& source) const {
        for (const auto& listener: m_listeners) {
            if (listener.get().interested_in(source)) {
                listener.get().drag_ended(source);
            }
        }
    }


    void notify_drag_start(const DragInfo& source) const {
        for (const auto& listener: m_listeners) {
            if (listener.get().interested_in(source)) {
                listener.get().drag_started(source);
            }
        }
    }


//    bool contains(const DragInfo& source) const {
//        return m_ongoing_drags.contains([&source](const std::unique_ptr<DragImageComponent>& drag_image) {
//            // TODO: Not sure if we're comparing pointers or references here, too many ampersands
//            return &drag_image->get_drag_info() == &source;
//        });
//    }
//
//    std::optional<std::size_t> index_of(const DragInfo& source) const {
//        return m_ongoing_drags.index([&source](const std::unique_ptr<DragImageComponent>& drag_image) {
//            return &drag_image->get_drag_info() == &source;
//        });
//    }

    Vec<std::reference_wrapper<DropListener>> m_listeners;

    const DragInfo* m_ongoing_drag = nullptr;

    std::unique_ptr<DragImageComponent> m_drag_image = nullptr;

    juce::WeakReference<DropAreaComponent> m_dragged_over_component;

};

class DragController {
public:
    DragController(juce::Component& snapshot_component, GlobalDragAndDropContainer& global_container)
            : m_snapshot_component(snapshot_component)
              , m_global_container(global_container) {}


    void start_drag(const DragInfo& source) {
        m_global_container.start_drag(source);
    }


    void cancel_drag(const DragInfo& source) {
        m_global_container.cancel_drag(source);
    }


    void finalize_drag(const DragInfo& source) {}


    void update_drag_image(const juce::ScaledImage& image) {}


private:
    juce::Component& m_snapshot_component;

    GlobalDragAndDropContainer& m_global_container;
};


#endif //SERIALISTLOOPER_DRAG_AND_DROP_H
