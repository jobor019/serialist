

#ifndef SERIALISTLOOPER_CONNECTOR_COMPONENT_H
#define SERIALISTLOOPER_CONNECTOR_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "component_utils.h"
#include "global_action_handler.h"
#include "core/socket_policy.h"

class ConnectorComponent : public juce::Component {
public:
    ConnectorComponent(juce::ValueTree socket_tree
                       , juce::Component& socket_component
                       , juce::Component& module_component
                       , juce::Component& parent)
            : m_socket_tree(std::move(socket_tree))
              , m_socket_component(socket_component)
              , m_module_component(module_component)
              , m_parent(parent)
              , m_registered_module(m_socket_tree.getProperty({Socket<int>::CONNECTED_PROPERTY}).toString()) {}


    void paint(juce::Graphics& g) override {
        auto socket_relative_bounds = bounds_relative_to_parent(m_socket_component);
        auto module_relative_bounds = bounds_relative_to_parent(m_module_component);

        int x0 = socket_relative_bounds.getX();
        int y0 = socket_relative_bounds.getY();
        int x1 = module_relative_bounds.getX();
        int y1 = module_relative_bounds.getY();

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
        auto socket_relative_bounds = bounds_relative_to_parent(m_socket_component);
        auto module_relative_bounds = bounds_relative_to_parent(m_module_component);

        auto socket_center = socket_relative_bounds.getCentre();
        auto module_corner = juce::Point<int>{module_relative_bounds.getCentreX()
                                              , module_relative_bounds.getBottom()};
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
        std::cout << "connected: " << get_connected_module() << ", registered: " << m_registered_module << " (eq: "
                  << (get_connected_module() == m_registered_module) << ")\n";
        return get_connected_module() == m_registered_module;
    }

    bool socket_equals(juce::Component& component) {
        return &m_socket_component == &component;
    }

    bool module_equals(juce::Component& component) {
        return &m_module_component == &component;
    }

    bool either_equals(juce::Component& component) {
        return socket_equals(component) || module_equals(component);
    }


private:

    juce::Rectangle<int> bounds_relative_to_parent(const Component& component) {
        return m_parent.getLocalArea(&component, component.getLocalBounds());
    }

    juce::String get_connected_module() {
        return m_socket_tree.getProperty({Socket<int>::CONNECTED_PROPERTY}).toString();
    }


    juce::ValueTree m_socket_tree;
    juce::Component& m_socket_component;
    juce::Component& m_module_component;
    juce::Component& m_parent;

    juce::String m_registered_module;
};


class ConnectionComponentManager : public juce::Component
                                   , public GlobalActionHandler::Listener
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

    void remove_connections_associated_with(juce::Component& component) {
        auto removed_components = ComponentUtils::get_named_children_recursively(&component);

        auto is_associated = [&removed_components](const std::unique_ptr<ConnectorComponent>& conn) {
            for (auto& removed_component : removed_components) {
               if (conn->either_equals(*removed_component)) {
                   return true;
               }
            }
            return false;
        };

        auto new_end = std::remove_if(m_connectors.begin(), m_connectors.end(), is_associated);
        auto changed = new_end != m_connectors.end();
        m_connectors.erase(new_end, m_connectors.end());

        if (changed) {
            resized();
        }
    }

    void on_action_change(Action * action) override {
        if (!action) {
            // TODO
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

            auto* connected_module = find_connected_module(socket_tree);
            if (!connected_module)
                return;


            std::cout << "TOP " << socket_widget->getTopLevelComponent()->getWidth() << ", "
                      << socket_widget->getTopLevelComponent()->getHeight() << "\n";
//            auto socket_bounds = socket_widget->getTopLevelComponent()->getLocalArea(socket_widget
//                                                                            , socket_widget->getLocalBounds());
//            auto module_bounds = connected_module->getTopLevelComponent()->getLocalArea(connected_module
//                                                                                , connected_module->getLocalBounds());

            auto socket_bounds = getLocalArea(socket_widget, socket_widget->getLocalBounds());
            auto module_bounds = getLocalArea(connected_module, connected_module->getLocalBounds());

//            auto socket_bounds =  socket_widget->localAreaToGlobal(socket_widget->getLocalBounds());
//            auto module_bounds =  socket_widget->localAreaToGlobal(socket_widget->getLocalBounds());

//            auto socket_bounds2 =  globa(socket_widget->getLocalBounds());
//            auto module_bounds2 =  socket_widget->localAreaToGlobal(socket_widget->getLocalBounds());


            std::cout << "socket_bounds GLOBAL: " << socket_bounds.getX() << ", " << socket_bounds.getY() << "\n";
            std::cout << "module_bounds GLOBAL: " << module_bounds.getX() << ", " << module_bounds.getY() << "\n";


            std::cout << "socket_bounds: " << socket_bounds.getX() << ", " << socket_bounds.getY() << "\n";
            std::cout << "module_bounds: " << module_bounds.getX() << ", " << module_bounds.getY() << "\n";
            auto connector = std::make_unique<ConnectorComponent>(socket_tree, *socket_widget, *connected_module, *this);
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
