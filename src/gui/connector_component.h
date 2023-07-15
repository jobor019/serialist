

#ifndef SERIALISTLOOPER_CONNECTOR_COMPONENT_H
#define SERIALISTLOOPER_CONNECTOR_COMPONENT_H

#include <juce_gui_extra/juce_gui_extra.h>
#include "component_utils.h"

class ConnectorComponent : public juce::Component {
public:

private:


};


class ConnectionComponentManager : public juce::Component
                                   , private juce::ValueTree::Listener {
public:
    explicit ConnectionComponentManager(juce::Component& parent, ParameterHandler& generative_parameter_handler)
            : m_parent(parent), m_generative_parameter_handler(generative_parameter_handler) {
        m_generative_parameter_handler.get_value_tree().addListener(this);
    }


    ~ConnectionComponentManager() override {
        m_generative_parameter_handler.get_value_tree().removeListener(this);
    }


    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged
                                  , const juce::Identifier& property) override {
        if (treeWhosePropertyHasChanged.getType() == ParameterKeys::GENERATIVE_SOCKET) {
            auto socket_name = treeWhosePropertyHasChanged.getProperty({ParameterKeys::ID_PROPERTY});
            auto connected_id = treeWhosePropertyHasChanged.getProperty({VTSocketBase<int>::CONNECTED_PROPERTY});
            auto socket_parent_tree = treeWhosePropertyHasChanged.getParent().getParent();

            if (socket_parent_tree.getType() != ParameterKeys::GENERATIVE)
                throw std::runtime_error("Invalid ValueTree structure");

            auto parent_id = socket_parent_tree.getProperty({ParameterKeys::ID_PROPERTY});

//            if (connected_id == parent_id) {} // TODO: This is not the condition.
//                                              //  It should look for the internal component id in the Widget/Module

            std::cout << "socket changed!!!: Connection from " << parent_id.toString() << "::" << socket_name.toString() << " to " << connected_id.toString() << "\n";
//            ComponentUtils::find_recursively(&m_parent, )
        }
    }


    void valueTreeChildRemoved(juce::ValueTree& parentTree
                               , juce::ValueTree& childWhichHasBeenRemoved
                               , int indexFromWhichChildWasRemoved) override {
    }


private:
    juce::Component& m_parent;
    ParameterHandler& m_generative_parameter_handler;

};

#endif //SERIALISTLOOPER_CONNECTOR_COMPONENT_H
