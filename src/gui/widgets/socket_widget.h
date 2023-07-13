

#ifndef SERIALISTLOOPER_SOCKET_WIDGET_H
#define SERIALISTLOOPER_SOCKET_WIDGET_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "socket_policy.h"
#include "generative_component.h"
#include "keyboard_shortcuts.h"
#include "connectable.h"
#include "interaction_visualizer.h"
#include "interaction_visualizations.h"

template<typename T>
class SocketWidget : public juce::Component
                     , public Connectable
                     , public juce::DragAndDropTarget
                     , private juce::ValueTree::Listener {
public:
    explicit SocketWidget(Socket<T>& socket, std::unique_ptr<GenerativeComponent> default_widget)
            : m_socket(socket)
              , m_default_widget(std::move(default_widget)) {




        if (!m_default_widget) {
            throw std::runtime_error("A default component must be provided for the socket");
        }

        m_socket.add_value_tree_listener(*this);
        addAndMakeVisible(*m_default_widget);
        addAndMakeVisible(m_interaction_visualizer);

        m_default_widget->addMouseListener(this, true);
    }


    ~SocketWidget() override {
        m_socket.remove_value_tree_listener(*this);
    }


    SocketWidget(const SocketWidget&) = delete;
    SocketWidget& operator=(const SocketWidget&) = delete;
    SocketWidget(SocketWidget&&) noexcept = default;
    SocketWidget& operator=(SocketWidget&&) noexcept = default;


    std::vector<std::unique_ptr<InteractionVisualization>> create_visualizations() {
        std::vector<std::unique_ptr<InteractionVisualization>> visualizations;
        visualizations.emplace_back(std::make_unique<ConnectVisualization>(*this));
        return visualizations;
    }


    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
//    bool is(const juce::DragAndDropTarget::SourceDetails& source_details) override {
        if (auto* source = dragSourceDetails.sourceComponent.get())
            return connectable_to(*source);
        return false;
    }


    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
//    void item_dropped(const juce::DragAndDropTarget::SourceDetails& source_details) override {
        std::cout << "item dropped on socket\n";
        if (auto* connectable = dynamic_cast<Connectable*>(dragSourceDetails.sourceComponent.get())) {
            connect(*connectable);
        }
    }


    void mouseDrag(const juce::MouseEvent&) override {
        juce::DragAndDropContainer* parent_drag_component =
                juce::DragAndDropContainer::findParentDragContainerFor(this);

        if (parent_drag_component && !parent_drag_component->isDragAndDropActive()) {
//            parent_drag_component->startDragging("src", this, juce::ScaledImage(createComponentSnapshot(juce::Rectangle<int>(1, 1))));
            parent_drag_component->startDragging("src", this, juce::ScaledImage(juce::Image(juce::Image::PixelFormat::RGB, 1, 1, true)));
        }
    }


    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails&) override {
        std::cout << "enter SOCKET\n";
        m_interaction_visualizer.set_drag_and_dropping(true);
    }


    void itemDragExit(const juce::DragAndDropTarget::SourceDetails&) override {
        std::cout << "exit SOCKET\n";
        m_interaction_visualizer.set_drag_and_dropping(false);
    }


    Socket<T>& get_socket() {
        return m_socket;
    }


    GenerativeComponent& get_internal() {
        return *m_default_widget;
    }


    bool connectable_to(juce::Component& component) override {
        if (auto* generative_component = dynamic_cast<GenerativeComponent*>(&component)) {
            return m_socket.is_connectable(generative_component->get_generative());
        }

        return false;
    }


    bool connect(Connectable& connectable) override {
        if (auto* generative_component = dynamic_cast<GenerativeComponent*>(&connectable)) {
            return m_socket.try_connect(generative_component->get_generative());
        }
        return false;
    }


    void set_layout(int layout) {
        get_internal().set_layout(layout);
    }


    void paint(juce::Graphics&) override {}


    void resized() override {
        m_default_widget->setBounds(getLocalBounds());
        m_interaction_visualizer.setBounds(getLocalBounds());
    }


private:
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override {
        std::cout << "[TODO] VT changed in SocketWidget\n";
    }


    Socket<T>& m_socket;

    std::unique_ptr<GenerativeComponent> m_default_widget;

    InteractionVisualizer m_interaction_visualizer{*this, create_visualizations()};

};

#endif //SERIALISTLOOPER_SOCKET_WIDGET_H
