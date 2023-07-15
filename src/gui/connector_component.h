

#ifndef SERIALISTLOOPER_CONNECTOR_COMPONENT_H
#define SERIALISTLOOPER_CONNECTOR_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "component_utils.h"

class ConnectorComponent : public juce::Component {
public:
    ConnectorComponent(juce::ValueTree socket_tree
                       , juce::Rectangle<int> socket_bounds
                       , juce::Rectangle<int> module_bounds)
            : m_socket_tree(std::move(socket_tree))
              , m_socket_bounds(socket_bounds)
              , m_module_bounds(module_bounds)
              , m_registered_module(m_socket_tree.getProperty({Socket<int>::CONNECTED_PROPERTY}).toString()) {}


    void paint(juce::Graphics& g) override {
        int x0 = m_socket_bounds.getX();
        int y0 = m_socket_bounds.getY();
        int x1 = m_module_bounds.getX();
        int y1 = m_module_bounds.getY();

        bool delta_x = x1 > x0;
        bool delta_y = y1 > y0;

        g.setColour(juce::Colours::darkorchid);

        if ((delta_x && delta_y) || (!delta_y && !delta_x)) {
            g.drawLine(0, 0, static_cast<float>(getWidth()), static_cast<float>(getHeight()));
        } else {
            g.drawLine(0, static_cast<float>(getHeight()), static_cast<float>(getWidth()), 0);
        }
    }


    juce::Rectangle<int> target_bounds() {
        auto socket_center = m_socket_bounds.getCentre();
        auto module_corner = juce::Point<int>{m_module_bounds.getCentreX(), m_module_bounds.getBottom()};
//        auto module_corner = m_module_bounds.getCentre();

        std::cout << "target bounds: \n"
                  << "  socket x: " << socket_center.getX()
                  << ", module x: " << module_corner.getX()
                  << "\n  socket y: " << socket_center.getY()
                  << ", module y: " << module_corner.getY() << "\n";

        int x = std::min(socket_center.getX(), module_corner.getX());
        int y = std::min(socket_center.getY(), module_corner.getY());
        int w = std::abs(socket_center.getX() - module_corner.getX());
        int h = std::abs(socket_center.getY() - module_corner.getY());
        return {x, y, w, h};
    }


    bool is_valid() {
        std::cout << "connected: " << get_connected_module() << ", registered: " << m_registered_module << " (eq: " << (get_connected_module() == m_registered_module) << ")\n";
        return get_connected_module() == m_registered_module;
    }


private:
    juce::String get_connected_module() {
        return m_socket_tree.getProperty({Socket<int>::CONNECTED_PROPERTY}).toString();
    }


    juce::ValueTree m_socket_tree;
    juce::Rectangle<int> m_socket_bounds;
    juce::Rectangle<int> m_module_bounds;

    juce::String m_registered_module;
};


class ConnectionComponentManager : public juce::Component
                                   , private juce::ValueTree::Listener {
public:
    explicit ConnectionComponentManager(juce::Component& parent, ParameterHandler& generative_parameter_handler)
            : m_parent(parent), m_generative_parameter_handler(generative_parameter_handler) {
        setWantsKeyboardFocus(false);
        setInterceptsMouseClicks(false, false);

        m_generative_parameter_handler.get_value_tree().addListener(this);
    }


    ~ConnectionComponentManager() override {
        m_generative_parameter_handler.get_value_tree().removeListener(this);
    }


    void resized() override {
        for (auto& connector: m_connectors) {
            connector->setBounds(connector->target_bounds());
        }
    }


    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier&) override {
        if (treeWhosePropertyHasChanged.getType() == ParameterKeys::GENERATIVE_SOCKET
            && treeWhosePropertyHasChanged.getProperty({Socket<int>::CONNECTED_PROPERTY}).toString().isNotEmpty()) {

            add_connection(treeWhosePropertyHasChanged);

//            auto* socket_widget = find_socket_widget_of(treeWhosePropertyHasChanged);
//            if (!socket_widget) {
//                throw std::runtime_error("Could not find the socket widget: invalid ValueTree structure");
//            }
//
//            auto* connected_component = find_connected_module(treeWhosePropertyHasChanged);
//            if (!connected_component) {
//                throw std::runtime_error("Could not find the connected component: invalid ValueTree structure");
//            }


//            std::cout << "socket changed!!!: Connection from " << parent_id << "::" << socket_name.toString()
//                      << " to " << connected_id.toString() << "\n";
        }
    }


    void valueTreeChildRemoved(juce::ValueTree&
                               , juce::ValueTree&
                               , int) override {
        if (remove_invalid_connections()) {
            resized();
        }
    }


private:

    void add_connection(juce::ValueTree& socket_tree) {
        if (socket_tree.getType() == ParameterKeys::GENERATIVE_SOCKET
            && socket_tree.getProperty({Socket<int>::CONNECTED_PROPERTY}).toString().isNotEmpty()) {

            remove_invalid_connections();


            auto* socket_widget = find_socket_widget_of(socket_tree);
            if (!socket_widget)
                return;

            auto* connected_component = find_connected_module(socket_tree);
            if (!connected_component)
                return;


            std::cout << "TOP " << socket_widget->getTopLevelComponent()->getWidth() << ", "
                      << socket_widget->getTopLevelComponent()->getHeight() << "\n";
//            auto socket_bounds = socket_widget->getTopLevelComponent()->getLocalArea(socket_widget
//                                                                            , socket_widget->getLocalBounds());
//            auto module_bounds = connected_component->getTopLevelComponent()->getLocalArea(connected_component
//                                                                                , connected_component->getLocalBounds());

            auto socket_bounds = getLocalArea(socket_widget, socket_widget->getLocalBounds());
            auto module_bounds = getLocalArea(connected_component, connected_component->getLocalBounds());

//            auto socket_bounds =  socket_widget->localAreaToGlobal(socket_widget->getLocalBounds());
//            auto module_bounds =  socket_widget->localAreaToGlobal(socket_widget->getLocalBounds());

//            auto socket_bounds2 =  globa(socket_widget->getLocalBounds());
//            auto module_bounds2 =  socket_widget->localAreaToGlobal(socket_widget->getLocalBounds());


            std::cout << "socket_bounds GLOBAL: " << socket_bounds.getX() << ", " << socket_bounds.getY() << "\n";
            std::cout << "module_bounds GLOBAL: " << module_bounds.getX() << ", " << module_bounds.getY() << "\n";


            std::cout << "socket_bounds: " << socket_bounds.getX() << ", " << socket_bounds.getY() << "\n";
            std::cout << "module_bounds: " << module_bounds.getX() << ", " << module_bounds.getY() << "\n";
            auto connector = std::make_unique<ConnectorComponent>(socket_tree, socket_bounds, module_bounds);
            addAndMakeVisible(*connector);
            m_connectors.push_back(std::move(connector));
            resized();
            std::cout << "Added connector (m_connections size: " << m_connectors.size() << "\n";
        }
    }


    bool remove_invalid_connections() {
        auto new_end = std::remove_if(m_connectors.begin()
                                      , m_connectors.end()
                                      , [](const auto& c) { return !c->is_valid(); });
        bool elements_removed = new_end != m_connectors.end();
        m_connectors.erase(new_end, m_connectors.end());

        return elements_removed;
    }


    juce::Component* find_socket_widget_of(juce::ValueTree& socket_tree) {
        auto socket_name = socket_tree.getProperty({ParameterKeys::ID_PROPERTY});
        auto parent_id = get_parent_id(socket_tree);

        auto socket_parent_trace = ComponentUtils::find_recursively_with_trace(&m_parent, parent_id);

        if (socket_parent_trace.empty()) {
            // Connection established before GenerativeComponent was added to ConfigurationComponent
            return nullptr;
        }

        return socket_parent_trace.at(0)->findChildWithID({socket_name});
    }


    juce::Component* find_connected_module(juce::ValueTree& socket_tree) {
        auto connected_id = socket_tree.getProperty({VTSocketBase<int>::CONNECTED_PROPERTY}).toString();

        return find_module(connected_id);

    }


    juce::Component* find_module(const juce::String& module_id) {
        return ComponentUtils::find_recursively(&m_parent, module_id);
    }


    static juce::String get_parent_id(juce::ValueTree& socket_tree) {
        auto socket_parent_tree = socket_tree.getParent().getParent();

        if (socket_parent_tree.getType() != ParameterKeys::GENERATIVE)
            throw std::runtime_error("Invalid ValueTree structure");
        return socket_parent_tree.getProperty({ParameterKeys::ID_PROPERTY});
    }


    juce::Component& m_parent;
    ParameterHandler& m_generative_parameter_handler;

    std::vector<std::unique_ptr<ConnectorComponent>> m_connectors;

};

#endif //SERIALISTLOOPER_CONNECTOR_COMPONENT_H
