

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

    class ConnectionSourceComponent : public juce::Component {
    public:
        void paint(juce::Graphics &g) override {
            g.fillAll(juce::Colours::slategrey);
        }
    };


    explicit SocketWidget(Socket<T>& socket, std::unique_ptr<GenerativeComponent> default_widget)
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


    ~SocketWidget() override {
        m_socket.remove_value_tree_listener(*this);
    }


    SocketWidget(const SocketWidget&) = delete;
    SocketWidget& operator=(const SocketWidget&) = delete;
    SocketWidget(SocketWidget&&) noexcept = default;
    SocketWidget& operator=(SocketWidget&&) noexcept = default;

    static bool is_connectable() {
        return GlobalKeyState::is_down_exclusive(ConfigurationLayerKeyboardShortcuts::CONNECTOR_KEY);
    }


    std::vector<std::unique_ptr<InteractionVisualization>> create_visualizations() {
        std::vector<std::unique_ptr<InteractionVisualization>> visualizations;
        visualizations.emplace_back(std::make_unique<ConnectVisualization>(*this));
        return visualizations;
    }


    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
        if (is_connectable()) {
            if (auto* source = dragSourceDetails.sourceComponent.get())
                return connectable_to(*source) && is_connectable();
        }
        return false;
    }


    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override {
        if (is_connectable()) {
            if (auto* connectable = dynamic_cast<Connectable*>(dragSourceDetails.sourceComponent.get())) {
                connect(*connectable);
            }
        }
    }


    void mouseDrag(const juce::MouseEvent&) override {
        juce::DragAndDropContainer* parent_drag_component =
                juce::DragAndDropContainer::findParentDragContainerFor(this);

        if (is_connectable() && parent_drag_component && !parent_drag_component->isDragAndDropActive()) {
            auto img = juce::Image(juce::Image::PixelFormat::RGB, 100, 30, true);
            juce::Graphics g (img);
            g.setColour (juce::Colours::steelblue);
//            g.fillRect(img.getBounds());
            g.setColour (juce::Colours::powderblue);
            img.multiplyAllAlphas(0.5f);
            g.drawFittedText("typename", img.getBounds(), juce::Justification::centred, 1);


            parent_drag_component->startDragging("src", this, juce::ScaledImage(img));
        }
    }


    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails&) override {
        if (is_connectable())
        m_interaction_visualizer.set_drag_and_dropping(true);
    }


    void itemDragExit(const juce::DragAndDropTarget::SourceDetails&) override {
        if (is_connectable())
        m_interaction_visualizer.set_drag_and_dropping(false);
    }


    Socket<T>& get_socket() {
        return m_socket;
    }


    GenerativeComponent& get_internal() {
        return *m_default_widget;
    }


    bool connectable_to(juce::Component& component) override {
        if (is_connectable()) {
            if (auto* generative_component = dynamic_cast<GenerativeComponent*>(&component)) {
                return m_socket.is_connectable(generative_component->get_generative());
            }
        }

        return false;
    }


    bool connect(Connectable& connectable) override {
        if (is_connectable()) {
            if (auto* generative_component = dynamic_cast<GenerativeComponent*>(&connectable)) {
                return m_socket.try_connect(generative_component->get_generative());
            }
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
    void valueTreePropertyChanged(juce::ValueTree& vt, const juce::Identifier& id) override {
        if (m_socket.equals_property(vt, id)) {
            std::cout << "SOCKETWIDGET VTP CHANGED\n";
            bool old_visibility = m_connection_source_component.isVisible();
            bool new_visibility;

            auto* connected = m_socket.get_connected();

            if (connected == nullptr) {
                std::cout << "RECONNECTING INTERNAL\n";
                m_socket.try_connect(m_default_widget->get_generative());
                new_visibility = false;
            } else {
                new_visibility = m_socket.get_connected() != &m_default_widget->get_generative();
            }

            m_connection_source_component.setVisible(new_visibility);
            if (old_visibility != new_visibility) {
                resized();
            }
        }
    }


    Socket<T>& m_socket;

    std::unique_ptr<GenerativeComponent> m_default_widget;

    InteractionVisualizer m_interaction_visualizer{*this, create_visualizations()};

    ConnectionSourceComponent m_connection_source_component;


};

#endif //SERIALISTLOOPER_SOCKET_WIDGET_H
