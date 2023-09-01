

#ifndef SERIALISTLOOPER_NOP_SOCKET_H
#define SERIALISTLOOPER_NOP_SOCKET_H

#include <string>

#include "generative.h"

template<typename SocketType>
class NopSocketBase {
public:
    NopSocketBase(const std::string& id, ParameterHandler& parent, SocketType* initial = nullptr) {
        (void) id;
        (void) parent;

        static_assert(std::is_base_of_v<Generative, SocketType>, "SocketType must inherit from Generative");

        m_node = initial;
    }


    void connect(SocketType& node) {
        m_node = &node;
    }


    void disconnect() {
        m_node = nullptr;

    }


    Generative* get_connected() const {
        return dynamic_cast<Generative*>(m_node);
    }


    bool is_connected() const {
        return m_node;
    }


    SocketType* m_node = nullptr;
};


// ==============================================================================================

template<typename T>
class NopSocket : public NopSocketBase<Node<T>> {
public:

    NopSocket(const std::string& id, ParameterHandler& parent, Node<T>* initial = nullptr)
            : NopSocketBase<Node<T>>(id, parent, initial) {}


    NopSocket& operator=(Node<T>* node) {
        if (node)
            NopSocketBase<Node<T>>::connect(*node);
        else
            NopSocketBase<Node<T>>::disconnect();
        return *this;
    }


    std::vector<T> process(const TimePoint& t) {
        if (NopSocketBase<Node<T>>::m_node == nullptr)
            return {};
        return NopSocketBase<Node<T>>::m_node->process(t);
    }


    T process_or(const TimePoint& t, T default_value) {
        if (!NopSocketBase<Node<T>>::m_node)
            return default_value;

        auto values = NopSocketBase<Node<T>>::m_node->process((t));

        if (values.empty())
            return default_value;
        else
            return values.at(0);
    }

};


// ==============================================================================================

template<typename T>
class NopDataSocket : public NopSocketBase<Leaf<T>> {
public:
    NopDataSocket(const std::string& id, ParameterHandler& parent, Leaf<T>* initial = nullptr)
            : NopSocketBase<Leaf<T>>(id, parent, initial) {}


    NopDataSocket& operator=(Leaf<T>* node) {
        if (node)
            NopSocketBase<Leaf<T>>::connect(*node);
        else
            NopSocketBase<Leaf<T>>::disconnect();
        return *this;
    }


    std::vector<T> process(const TimePoint& t, double y, InterpolationStrategy strategy) {
        if (NopSocketBase<Leaf<T>>::m_node == nullptr)
            return {};
        return NopSocketBase<Leaf<T>>::m_node->process(t, y, strategy);
    }


};


#endif //SERIALISTLOOPER_NOP_SOCKET_H
