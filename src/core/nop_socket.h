

#ifndef SERIALISTLOOPER_NOP_SOCKET_H
#define SERIALISTLOOPER_NOP_SOCKET_H

#include <string>

#include "generative.h"
#include "connectable.h"

template<typename SocketType>
class NopSocketBase : public Connectable {
public:
    NopSocketBase(const std::string&, ParameterHandler&, SocketType* initial = nullptr) {
        static_assert(std::is_base_of_v<Generative, SocketType>, "SocketType must inherit from Generative");

        m_node = initial;
    }


    Generative* get_connected() const override {
        return dynamic_cast<Generative*>(m_node);
    }


    bool is_connected() const override {
        return m_node;
    }


    bool is_connectable(Generative& generative) const override {
        return static_cast<bool>(dynamic_cast<SocketType*>(&generative));
    }


    bool try_connect(Generative& generative) override {
        if (auto* node = dynamic_cast<SocketType*>(&generative)) {
            set_connection_internal(node);
            return true;
        }
        return false;
    }


    void disconnect_if(Generative& connected_to) override {
        if (get_connected() == &connected_to) {
            set_connection_internal(nullptr);
        }
    }


    void connect(SocketType& node) {
        set_connection_internal(&node);
    }


    void disconnect() {
        set_connection_internal(nullptr);

    }


protected:
    void set_connection_internal(SocketType* node) {
        if (node) {
            m_node = node;
        } else {
            m_node = nullptr;
        }
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


    Voices<T> process() {
        if (NopSocketBase<Node<T>>::m_node == nullptr)
            return Voices<T>::create_empty_like();
        return NopSocketBase<Node<T>>::m_node->process();
    }


    Voices<T> process(std::size_t num_voices) {
        return process().adapted_to(num_voices);
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


    std::vector<T> process(double y, InterpolationStrategy strategy) {
        if (NopSocketBase<Leaf<T>>::m_node == nullptr)
            return {};
        return NopSocketBase<Leaf<T>>::m_node->process(y, strategy);
    }


};


#endif //SERIALISTLOOPER_NOP_SOCKET_H
