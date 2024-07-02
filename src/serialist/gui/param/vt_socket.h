

#ifndef SERIALISTLOOPER_VT_SOCKET_H
#define SERIALISTLOOPER_VT_SOCKET_H

#include <string>
#include <thread>
#include <utility>

#include "core/generative.h"
#include "core/connectable.h"
#include "core/param/socket_base.h"

namespace serialist {

template<typename T>
class VTSocket : public SocketBase<T>
                 , private juce::ValueTree::Listener {
public:
    static const inline std::string CONNECTED_PROPERTY = "connected";


    VTSocket(std::string id, ParameterHandler& parent, Node<T>* initial = nullptr)
            : m_id(std::move(id)), m_parent(parent) {

        if (!m_parent.get_value_tree().isValid())
            throw ParameterError("Cannot register VTParameterBase for invalid tree");

        m_value_tree = juce::ValueTree({ParameterTypes::GENERATIVE_SOCKET});
        m_value_tree.setProperty({ParameterTypes::ID_PROPERTY}, {m_id}, &m_parent.get_undo_manager());
        m_parent.get_value_tree().addChild(m_value_tree, -1, &m_parent.get_undo_manager());

        set_connection_internal(initial);
        m_value_tree.addListener(this);
    }


    ~VTSocket() override {
        m_parent.get_value_tree().removeChild(m_value_tree, &m_parent.get_undo_manager());
    }


    VTSocket(const VTSocket&) = delete;
    VTSocket& operator=(const VTSocket&) = delete;
    VTSocket(VTSocket&&) noexcept = default;
    VTSocket& operator=(VTSocket&&) noexcept = default;

    VTSocket<T>& operator=(Node<T>* node) {
        if (node)
            SocketBase<T>::connect(*node);
        else
            SocketBase<T>::disconnect();
        return *this;
    }


    void add_value_tree_listener(juce::ValueTree::Listener& listener) {
        m_parent.get_value_tree().addListener(&listener);
    }


    void remove_value_tree_listener(juce::ValueTree::Listener& listener) {
        m_parent.get_value_tree().removeListener(&listener);
    }


    bool equals_property(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier&) {
        return treeWhosePropertyHasChanged == m_value_tree;
    }


    [[nodiscard]] juce::Identifier get_identifier() const {
        return {m_id};
    }


    [[nodiscard]] std::string get_identifier_as_string() const {
        return get_identifier().toString().toStdString();
    }


private:
    void set_connection_internal(Node<T>* node) override {
        if (node) {
            SocketBase<T>::set_node(node);
            update_value_tree(node->get_parameter_handler().get_id());
        } else {
            SocketBase<T>::set_node(nullptr);
            update_value_tree("");
        }
    }


    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override {
        std::cout << "VT changed: TODO update internal state\n";
    }


    void update_value_tree(const juce::String& connected_node_identifier) {
        m_value_tree.setProperty({CONNECTED_PROPERTY}
                                 , connected_node_identifier
                                 , &m_parent.get_undo_manager());
    }


    std::string m_id;
    VTParameterHandler& m_parent;

    juce::ValueTree m_value_tree;
};


} // namespace serialist

#endif //SERIALISTLOOPER_VT_SOCKET_H
