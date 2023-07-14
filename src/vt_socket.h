

#ifndef SERIALISTLOOPER_VT_SOCKET_H
#define SERIALISTLOOPER_VT_SOCKET_H

#include <string>

#include "generative.h"

template<typename SocketType>
class VTSocketBase : private juce::ValueTree::Listener {

    static const inline std::string CONNECTED_PROPERTY = "connected";


public:
    VTSocketBase(const std::string& id, ParameterHandler& parent, SocketType* initial = nullptr)
            : m_id(id), m_parent(parent) {

        static_assert(std::is_base_of_v<Generative, SocketType>, "SocketType must inherit from Generative");

        if (!m_parent.get_value_tree().isValid())
            throw ParameterError("Cannot register VTParameter for invalid tree");

        m_value_tree = juce::ValueTree({ParameterKeys::GENERATIVE_SOCKET});
        m_value_tree.setProperty({ParameterKeys::ID_PROPERTY}, {m_id}, &m_parent.get_undo_manager());
        m_parent.get_value_tree().addChild(m_value_tree, -1, &m_parent.get_undo_manager());

        set_connection_internal(initial);
        m_value_tree.addListener(this);
    }


    ~VTSocketBase() override {
        m_parent.get_value_tree().removeChild(m_value_tree, &m_parent.get_undo_manager());
    }


    bool is_connectable(Generative& generative) {
        std::lock_guard<std::mutex> lock{m_mutex};
        return static_cast<bool>(dynamic_cast<SocketType*>(&generative));
    }


    bool try_connect(Generative& generative) {
        if (auto* node = dynamic_cast<SocketType*>(&generative)) {
            set_connection_internal(node);
            return true;
        }
        return false;
    }


    void connect(SocketType& node) {
        std::lock_guard<std::mutex> lock{m_mutex};
        set_connection_internal(&node);
    }


    void disconnect() {
        std::lock_guard<std::mutex> lock{m_mutex};
        set_connection_internal(nullptr);

    }


    [[nodiscard]] Generative* get_connected() const {
        return dynamic_cast<Generative*>(m_node);
    }


    [[nodiscard]] bool is_connected() const {
        return m_node;
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


protected:
    void set_connection_internal(SocketType* node) {
        if (node) {
            m_node = node;
            update_value_tree(node->get_parameter_handler().get_identifier().toString());
        } else {
            m_node = nullptr;
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


    std::mutex m_mutex;

    std::string m_id;
    VTParameterHandler& m_parent;

    juce::ValueTree m_value_tree;

    SocketType* m_node = nullptr;
};


// ==============================================================================================

template<typename T>
class VTSocket : public VTSocketBase<Node<T>> {
public:

    VTSocket(const std::string& id, ParameterHandler& parent, Node<T>* initial = nullptr)
            : VTSocketBase<Node<T>>(id, parent, initial) {}


    VTSocket& operator=(Node<T>* node) {
        std::lock_guard<std::mutex> lock{VTSocketBase<Node<T>>::m_mutex};
        VTSocketBase<Node<T>>::set_connection_internal(node);
        return *this;
    }


    std::vector<T> process(const TimePoint& t) {
        std::lock_guard<std::mutex> lock{VTSocketBase<Node<T>>::m_mutex};
        if (VTSocketBase<Node<T>>::m_node == nullptr)
            return {};
        return VTSocketBase<Node<T>>::m_node->process(t);
    }


    T process_or(const TimePoint& t, T default_value) {
        std::lock_guard<std::mutex> lock{VTSocketBase<Node<T>>::m_mutex};
        if (!VTSocketBase<Node<T>>::m_node)
            return default_value;

        auto values = VTSocketBase<Node<T>>::m_node->process((t));

        if (values.empty())
            return default_value;
        else
            return values.at(0);
    }

};


// ==============================================================================================

template<typename T>
class VTDataSocket : public VTSocketBase<DataNode<T>> {
public:
    VTDataSocket(const std::string& id, ParameterHandler& parent, DataNode<T>* initial = nullptr)
            : VTSocketBase<DataNode<T>>(id, parent, initial) {}


    VTDataSocket& operator=(DataNode<T>* node) {
        std::lock_guard<std::mutex> lock{VTSocketBase<DataNode<T>>::m_mutex};
        VTSocketBase<DataNode<T>>::set_connection_internal(node);
        return *this;
    }


    std::vector<T> process(const TimePoint& t, double y, InterpolationStrategy<T> strategy) {
        std::lock_guard<std::mutex> lock{VTSocketBase<DataNode<T>>::m_mutex};
        if (VTSocketBase<DataNode<T>>::m_node == nullptr)
            return {};
        return VTSocketBase<DataNode<T>>::m_node->process(t, y, strategy);
    }


};

#endif //SERIALISTLOOPER_VT_SOCKET_H
