
#ifndef SERIALISTLOOPER_DRAG_AND_DROP_H
#define SERIALISTLOOPER_DRAG_AND_DROP_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "core/collections/vec.h"

class DragInfo {
public:
    DragInfo() = default;
    virtual ~DragInfo() = default;
    DragInfo(const DragInfo&) = delete;
    DragInfo& operator=(const DragInfo&) = delete;
    DragInfo(DragInfo&&) noexcept = default;
    DragInfo& operator=(DragInfo&&) noexcept = default;
};


class DragImageComponent : public juce::Component {
public:
    explicit DragImageComponent(const juce::ScaledImage& image
                                , const juce::Point<int>& source_mouse_down_position
                                , const juce::Point<int>& dnd_container_mouse_down_position)
                                : m_image(image)
                                , m_mouse_down_offset(source_mouse_down_position) {
        setInterceptsMouseClicks(false, false);

        const auto bounds = image.getScaledBounds().toNearestInt();
        setSize (bounds.getWidth(), bounds.getHeight());
        update_position(dnd_container_mouse_down_position);
    }


    void paint(juce::Graphics& g) override {
        g.drawImage (m_image.getImage(), m_image.getScaledBounds().toFloat());
    }

    void update_position(const juce::Point<int>& container_mouse_position) {
        setTopLeftPosition(container_mouse_position - m_mouse_down_offset);
    }


private:
    juce::ScaledImage m_image;
    const juce::Point<int> m_mouse_down_offset;

};


class DropListener {
public:
    DropListener() = default;
    virtual ~DropListener() = default;
    DropListener(const DropListener&) = delete;
    DropListener& operator=(const DropListener&) = delete;
    DropListener(DropListener&&) noexcept = default;
    DropListener& operator=(DropListener&&) noexcept = default;

    virtual bool interested_in(const DragInfo& source) = 0;

    virtual void external_dnd_action_started(const DragInfo& source) = 0;
    virtual void external_dnd_action_ended(const DragInfo& source) = 0;

    virtual void drop_enter(const DragInfo& source, const juce::MouseEvent& event) = 0;
    virtual void drop_exit(const DragInfo& source) = 0;
    virtual void drop_move(const DragInfo& source, const juce::MouseEvent& event) = 0;

    virtual void item_dropped(const DragInfo& source) = 0;

};


class DropArea {
public:
    DropArea() = default;
    virtual ~DropArea() = default;
    DropArea(const DropArea&) = delete;
    DropArea& operator=(const DropArea&) = delete;
    DropArea(DropArea&&) noexcept = delete;
    DropArea& operator=(DropArea&&) noexcept = delete;

    virtual DropListener& get_drop_listener() = 0;

private:
    JUCE_DECLARE_WEAK_REFERENCEABLE(DropArea)
};


class GlobalDragAndDropContainer : public juce::Component {
public:
    explicit GlobalDragAndDropContainer(juce::Component& root_component) : m_root_component(root_component) {
        setInterceptsMouseClicks(false, false);
        m_root_component.addMouseListener(this, true);
    }


    ~GlobalDragAndDropContainer() override {
        m_root_component.removeMouseListener(this);
    }

    GlobalDragAndDropContainer(const GlobalDragAndDropContainer&) = delete;
    GlobalDragAndDropContainer& operator=(const GlobalDragAndDropContainer&) = delete;
    GlobalDragAndDropContainer(GlobalDragAndDropContainer&&)  noexcept = delete;
    GlobalDragAndDropContainer& operator=(GlobalDragAndDropContainer&&)  noexcept = delete;


    void start_drag(const DragInfo& source, const juce::ScaledImage& drag_image, const juce::MouseEvent& event) {
        assert(m_ongoing_drag == nullptr);
        assign_drag(&source, std::make_unique<DragImageComponent>(drag_image
                                                                  , event.getPosition()
                                                                  , event.getEventRelativeTo(this).getPosition()));
        std::cout << "Drag start\n";

        if (auto target = find_target(event)) {
            m_dragged_over_component = target;
            target->get_drop_listener().drop_enter(source, event);
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
//                target->get_drop_listener().drop_enter(source);
//            }
//        }
    }


    void cancel_drag(const DragInfo& source) {
        if (is_dragging(source)) {
            if (has_valid_drop_target()) {
                std::cout << "notifying listener on cancel\n";
                m_dragged_over_component->get_drop_listener().drop_exit(source);
            }

            m_dragged_over_component = nullptr;

            assign_drag(nullptr, nullptr);
        }

//        if (auto index = index_of(source)) {
//            if (!m_dragged_over_component.wasObjectDeleted()) {
//                m_dragged_over_component->get_drop_listener().drop_exit(source);
//                m_dragged_over_component = nullptr;
//            }
//
//            if (auto drag_image = m_ongoing_drags.pop_index(*index)) {
//                for (const auto& listener: m_listeners) {
//                    listener.get().external_dnd_action_ended(source);
//                }
//            }
//        }
    }


    void finalize_drag(const DragInfo& source) {
        if (is_dragging(source)) {
            if (has_valid_drop_target()) {
                m_dragged_over_component->get_drop_listener().item_dropped(source);
            }

            m_dragged_over_component = nullptr;

            assign_drag(nullptr, nullptr);
        }


//        if (auto index = index_of(source)) {
//            if (!m_dragged_over_component.wasObjectDeleted()) {
//                m_dragged_over_component->get_drop_listener().item_dropped(source);
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


    void remove_listener(const DropListener& listener) noexcept {
        m_listeners.remove([&listener](const auto& l) { return &l.get() == &listener; });
    }


    bool is_dragging() const {
        return static_cast<bool>(m_ongoing_drag);
    }


    bool is_dragging(const DragInfo& source) const {
        return m_ongoing_drag == &source;
    }


    bool has_valid_drop_target() const {
        return static_cast<bool>(m_dragged_over_component);
    }


    void mouseDrag(const juce::MouseEvent& event) override {
        if (!is_dragging()) {
            return;
        }

        if (m_drag_image) {
            m_drag_image->update_position(event.getEventRelativeTo(this).getPosition());
        }

        if (auto target = find_target(event)) {

            if (target == m_dragged_over_component.get()) {
                m_dragged_over_component->get_drop_listener().drop_move(*m_ongoing_drag, event);

            } else {
                // exit current component if existing
                if (has_valid_drop_target()) {
                    std::cout << "internal drop exit\n";
                    m_dragged_over_component->get_drop_listener().drop_exit(*m_ongoing_drag);
                }

                m_dragged_over_component = target;

                // if entering a new component, notify it
                if (has_valid_drop_target()) {
                    std::cout << "internal drop enter\n";
                    m_dragged_over_component->get_drop_listener().drop_enter(*m_ongoing_drag, event);
                }
            }
        }

    }

    // mouseUp etc. is NOT the responsibility of the GlobalDragAndDropContainer, it is handled by the DragController

    // For other listeners like the GUI visualizing ongoing connections between components
    DropArea* get_dragged_over_component() const {
        return m_dragged_over_component.get();
    }


private:
    DropArea* find_target(const juce::MouseEvent& event) {
        auto* component = m_root_component
                .getComponentAt(event.getEventRelativeTo(&m_root_component)
                .getPosition());

        while (component && component != &m_root_component) {
            if (auto* drop_area_component = dynamic_cast<DropArea*>(component)) {
                if (drop_area_component->get_drop_listener().interested_in(*m_ongoing_drag)) {
                    return drop_area_component;
                }
            }

            component = component->getParentComponent();
        }

        return nullptr;
    }


    void assign_drag(const DragInfo* source, std::unique_ptr<DragImageComponent> drag_image) {
        if (source) {
            m_drag_image = std::move(drag_image);
            addAndMakeVisible(*m_drag_image);
            m_ongoing_drag = source;
            notify_drag_start(*source);
            std::cout << "Assign ON\n";

        } else {
            if (m_ongoing_drag)
                notify_drag_end(*m_ongoing_drag);

            m_ongoing_drag = nullptr;
            m_drag_image = nullptr;
            std::cout << "Assign OFF\n";
        }

        resized();
    }


    void notify_drag_end(const DragInfo& source) const {
        for (const auto& listener: m_listeners) {
            if (listener.get().interested_in(source)) {
                listener.get().external_dnd_action_ended(source);
            }
        }
    }


    void notify_drag_start(const DragInfo& source) const {
        for (const auto& listener: m_listeners) {
            if (listener.get().interested_in(source)) {
                listener.get().external_dnd_action_started(source);
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


    juce::Component& m_root_component;

    Vec<std::reference_wrapper<DropListener>> m_listeners;

    const DragInfo* m_ongoing_drag = nullptr;

    std::unique_ptr<DragImageComponent> m_drag_image = nullptr;

    juce::WeakReference<DropArea> m_dragged_over_component;

};

class DragController {
public:
    DragController(juce::Component* default_snapshot_component, GlobalDragAndDropContainer& global_container)
            : m_default_snapshot_component(default_snapshot_component)
              , m_global_container(global_container) {}


    void start_drag(std::unique_ptr<DragInfo>&& source
                    , const juce::MouseEvent& mouse_event
                    , const std::optional<juce::ScaledImage>& snapshot) {
        if (m_ongoing_drag) {
            cancel_drag();
        }

        m_ongoing_drag = std::move(source);
        m_global_container.start_drag(*m_ongoing_drag, get_drag_image(snapshot), mouse_event);
    }


    void cancel_drag() {
        if (m_ongoing_drag) {
            std::cout << "Drag controller: cancel drag\n";
            m_global_container.cancel_drag(*m_ongoing_drag);
            m_ongoing_drag = nullptr;
        }
    }


    void finalize_drag() {
        if (m_ongoing_drag) {
            m_global_container.finalize_drag(*m_ongoing_drag);
            m_ongoing_drag = nullptr;
        }
    }


    void update_drag_image(const juce::ScaledImage& image) {
        (void) image;
        // TODO
    }


private:
    juce::ScaledImage get_drag_image(const std::optional<juce::ScaledImage>& snapshot) {
        if (snapshot) {
            return snapshot.value();
        } else if (m_default_snapshot_component) {
            return juce::ScaledImage(m_default_snapshot_component->
                    createComponentSnapshot(m_default_snapshot_component->getLocalBounds()));
        } else {
            return {};
        }
    }


    juce::Component* m_default_snapshot_component;
    std::unique_ptr<DragInfo> m_ongoing_drag;

    GlobalDragAndDropContainer& m_global_container;
};


#endif //SERIALISTLOOPER_DRAG_AND_DROP_H
