

#ifndef SERIALISTLOOPER_SOCKET_WIDGET_H
#define SERIALISTLOOPER_SOCKET_WIDGET_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "core/param/socket_policy.h"
#include "state/generative_component.h"
#include "keyboard_shortcuts.h"
#include "bases/connectable_module.h"
#include "bases/connectable_dnd_controller.h"

template<typename SocketType>
class TemplateSocketWidget : public juce::Component
                             , public ConnectableModule
                             , public juce::DragAndDropTarget
                             , private juce::ValueTree::Listener {
public:

    class ConnectionSourceComponent : public juce::Component {
    public:
        void paint(juce::Graphics& g) override {
            g.fillAll(juce::Colours::slategrey);
        }
    };


    explicit TemplateSocketWidget(SocketType& socket, std::unique_ptr<GenerativeComponent> default_widget)
            : m_socket(socket)
              , m_default_widget(std::move(default_widget)) {
        if (!m_default_widget) {
            throw std::runtime_error("A default component must be provided for the socket");
        }

        setComponentID(socket.get_identifier_as_string());

        m_socket.add_value_tree_listener(*this);
        addAndMakeVisible(*m_default_widget);
        addChildComponent(m_connection_source_component);
        addAndMakeVisible(m_interaction_visualizer);

        m_default_widget->addMouseListener(this, true);
    }


    ~TemplateSocketWidget() override {
        m_socket.remove_value_tree_listener(*this);
    }


    TemplateSocketWidget(const TemplateSocketWidget&) = delete;
    TemplateSocketWidget& operator=(const TemplateSocketWidget&) = delete;
    TemplateSocketWidget(TemplateSocketWidget&&) noexcept = default;
    TemplateSocketWidget& operator=(TemplateSocketWidget&&) noexcept = default;


    static bool is_disconnectable() {
        return GlobalKeyState::is_down_exclusive(ConfigurationLayerKeyboardShortcuts::DISCONNECT_KEY);
    }


    std::vector<std::unique_ptr<InteractionVisualization_LEGACY>> create_visualizations() {
        std::vector<std::unique_ptr<InteractionVisualization_LEGACY>> visualizations;
        visualizations.emplace_back(std::make_unique<DisconnectVisualization>(*this));
        return visualizations;
    }


    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
        return m_connectable_dnd_controller.is_interested_in(dragSourceDetails);
    }


    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
        m_connectable_dnd_controller.item_dropped(dragSourceDetails);
    }


    void mouseUp(const juce::MouseEvent&) override {
        if (is_disconnectable()) {
            if (!is_connected_to_internal()) {
                connect_internal();
            }
        }
    }


    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
        m_connectable_dnd_controller.item_drag_enter(dragSourceDetails);
    }


    void itemDragExit(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
        m_connectable_dnd_controller.item_drag_exit(dragSourceDetails);
    }


    SocketType& get_socket() {
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


    bool connect(ConnectableModule& connectable) override {
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
        m_connection_source_component.setBounds(getLocalBounds().removeFromTop(getHeight() / 2));
        m_interaction_visualizer.setBounds(getLocalBounds());
    }


private:
    bool is_connected_to_internal() {
        return m_socket.get_connected() == &m_default_widget->get_generative();
    }


    void connect_internal() {
        m_socket.try_connect(m_default_widget->get_generative());
    }


    void valueTreePropertyChanged(juce::ValueTree& vt, const juce::Identifier& id) override {
        if (m_socket.equals_property(vt, id)) {
            std::cout << "SOCKETWIDGET VTP CHANGED\n";
            bool old_visibility = m_connection_source_component.isVisible();
            bool new_visibility;

            auto* connected = m_socket.get_connected();

            if (connected == nullptr) {
                std::cout << "RECONNECTING INTERNAL\n";
                connect_internal();
                new_visibility = false;
            } else {
                new_visibility = !is_connected_to_internal();
            }

            m_connection_source_component.setVisible(new_visibility);
            if (old_visibility != new_visibility) {
                resized();
            }
        }
    }


    SocketType& m_socket;

    std::unique_ptr<GenerativeComponent> m_default_widget;

    InteractionVisualizer_LEGACY m_interaction_visualizer{*this, create_visualizations()};
    ConnectableDndController m_connectable_dnd_controller{*this, *this, &m_interaction_visualizer};


    ConnectionSourceComponent m_connection_source_component;

};


// ==============================================================================================

template<typename OutputType>
class SocketWidget : public TemplateSocketWidget<Socket<OutputType>> {
public:
    explicit SocketWidget(Socket<OutputType>& socket, std::unique_ptr<GenerativeComponent> default_widget)
            : TemplateSocketWidget<Socket<OutputType>>(socket, std::move(default_widget)) {}
};


// ==============================================================================================

template<typename OutputType>
class DataSocketWidget : public TemplateSocketWidget<DataSocket<OutputType>> {
public:
    explicit DataSocketWidget(DataSocket<OutputType>& socket, std::unique_ptr<GenerativeComponent> default_widget)
            : TemplateSocketWidget<DataSocket<OutputType>>(socket, std::move(default_widget)) {}
};


#endif //SERIALISTLOOPER_SOCKET_WIDGET_H
