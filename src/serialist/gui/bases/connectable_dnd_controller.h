
#ifndef SERIALISTLOOPER_CONNECTABLE_DND_CONTROLLER_H
#define SERIALISTLOOPER_CONNECTABLE_DND_CONTROLLER_H

#include "connectable_module.h"
#include "core/generative.h"

// TODO: Needs updates

class ConnectableDndController : public juce::MouseListener {
public:
//    ConnectableDndController(ConnectableModule& connectable
//                             , juce::Component& source_component
//                             , InteractionVisualizer_LEGACY* parent_interaction_visualizer)
//            : m_connectable(connectable)
//              , m_source_component(source_component)
//              , m_interaction_visualizer(parent_interaction_visualizer) {
//
//        parent_interaction_visualizer->add_visualization(
//                std::make_unique<ConnectVisualization>(m_source_component));
//
//        m_source_component.addMouseListener(this, true);
//    }
//
//
//    static bool is_connectable() {
//        return GlobalKeyState::is_down_exclusive(ConfigurationLayerKeyboardShortcuts::CONNECTOR_KEY);
//    }
//
//
//    bool is_interested_in(const juce::DragAndDropTarget::SourceDetails& source_details) {
//        if (is_connectable()) {
//            if (auto* source = source_details.sourceComponent.get()) {
//                return m_connectable.connectable_to(*source);
//            }
//        }
//        return false;
//    }
//
//
//    void item_dropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) {
//        if (is_connectable()) {
//            if (auto* connectable = dynamic_cast<ConnectableModule*>(dragSourceDetails.sourceComponent.get())) {
//                m_connectable.connect(*connectable);
//            }
//        }
//    }
//
//
//    void item_drag_enter(const juce::DragAndDropTarget::SourceDetails&) {
//        if (is_connectable() && m_interaction_visualizer)
//            m_interaction_visualizer->set_drag_and_dropping(true);
//    }
//
//
//    void item_drag_exit(const juce::DragAndDropTarget::SourceDetails&) {
//        if (is_connectable() && m_interaction_visualizer)
//            m_interaction_visualizer->set_drag_and_dropping(false);
//    }
//
//
//    void mouseDrag(const juce::MouseEvent&) override {
//        if (!is_connectable()) {
//            return;
//        }
//
//        juce::DragAndDropContainer* parent_drag_component =
//                juce::DragAndDropContainer::findParentDragContainerFor(&m_source_component);
//
//        if (parent_drag_component && !parent_drag_component->isDragAndDropActive()) {
//            auto img = juce::Image(juce::Image::PixelFormat::RGB, 100, 30, true);
//            juce::Graphics g(img);
//            g.setColour(juce::Colours::steelblue);
//            g.setColour(juce::Colours::powderblue);
//            img.multiplyAllAlphas(0.5f);
//            g.drawFittedText("typename", img.getBounds(), juce::Justification::centred, 1);
//            parent_drag_component->startDragging("src", &m_source_component, juce::ScaledImage(img));
//
//            // Old approach:
////                parent_drag_component->startDragging("connection", &m_source_component, juce::ScaledImage(
////                        m_source_component.createComponentSnapshot(juce::Rectangle<int>(1, 1))));
//        }
//
//    }
//
//
//private:
//    ConnectableModule& m_connectable;
//    juce::Component& m_source_component;
//
//    InteractionVisualizer_LEGACY* m_interaction_visualizer;

};

#endif //SERIALISTLOOPER_CONNECTABLE_DND_CONTROLLER_H
