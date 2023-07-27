
#ifndef SERIALISTLOOPER_CONNECTABLE_DND_CONTROLLER_H
#define SERIALISTLOOPER_CONNECTABLE_DND_CONTROLLER_H

#include "interaction_visualizer.h"
#include "connectable.h"
#include "generative.h"
#include "interaction_visualizations.h"

class ConnectableDndController : public juce::MouseListener {
public:
    ConnectableDndController(Connectable& connectable
                             , juce::Component& source_component
                             , InteractionVisualizer* parent_interaction_visualizer)
            : m_connectable(connectable)
              , m_source_component(source_component)
              , m_interaction_visualizer(parent_interaction_visualizer) {

        parent_interaction_visualizer->add_visualization(
                std::make_unique<ConnectVisualization>(m_source_component));

        m_source_component.addMouseListener(this, true);
    }


    bool is_interested_in(const juce::DragAndDropTarget::SourceDetails& source_details) {
        std::cout << "big interest " << (TEMP++) << "\n";
        if (auto* source = source_details.sourceComponent.get()) {
            return m_connectable.connectable_to(*source);
        }
        return false;
    }


    void item_dropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) {
        if (auto* connectable = dynamic_cast<Connectable*>(dragSourceDetails.sourceComponent.get())) {
            m_connectable.connect(*connectable);
        }
    }


    void item_drag_enter(const juce::DragAndDropTarget::SourceDetails&) {
        std::cout << "big interest\n";
        if (m_interaction_visualizer)
            m_interaction_visualizer->set_drag_and_dropping(true);
    }


    void item_drag_exit(const juce::DragAndDropTarget::SourceDetails&) {
        std::cout << "big interest\n";
        if (m_interaction_visualizer)
            m_interaction_visualizer->set_drag_and_dropping(false);
    }


    void mouseDrag(const juce::MouseEvent&) override {
        if (GlobalKeyState::is_down_exclusive(ConfigurationLayerKeyboardShortcuts::CONNECTOR_KEY)) {
            juce::DragAndDropContainer* parent_drag_component =
                    juce::DragAndDropContainer::findParentDragContainerFor(&m_source_component);

            if (parent_drag_component && !parent_drag_component->isDragAndDropActive()) {
                std::cout << "draggging\n";
                parent_drag_component->startDragging("connection", &m_source_component, juce::ScaledImage(
                        m_source_component.createComponentSnapshot(juce::Rectangle<int>(1, 1))));
            }
        }
    }


private:
    Connectable& m_connectable;
    juce::Component& m_source_component;

    InteractionVisualizer* m_interaction_visualizer;

    int TEMP = 0;

};

#endif //SERIALISTLOOPER_CONNECTABLE_DND_CONTROLLER_H
