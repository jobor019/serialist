
#ifndef SERIALISTLOOPER_SOCKET_BASE_H
#define SERIALISTLOOPER_SOCKET_BASE_H

#include "connectable.h"

template<typename T>
class SocketBase : public Connectable {
public:
    explicit SocketBase(Node<T>* initial = nullptr)
            : m_node(initial) {}


    Voices<T> process() {
        std::lock_guard lock{m_mutex};
        if (m_node == nullptr)
            return Voices<T>::create_empty_like();
        return m_node->process();
    }


    Voices<T> process(std::size_t num_voices) {
        return process().adapted_to(num_voices);
    }


    Generative* get_connected() const override {
        return dynamic_cast<Generative*>(m_node);
    }


    bool is_connectable(Generative& generative) const override {
        return static_cast<bool>(dynamic_cast<Node<T>*>(&generative));
    }


    bool is_connected() const override {
        return m_node;
    }


    bool try_connect(Generative& generative) override {
        std::lock_guard lock{m_mutex};
        if (auto* node = dynamic_cast<Node<T>*>(&generative)) {
            set_connection_internal(node);
            return true;
        }
        return false;
    }


    void disconnect_if(Generative& connected_to) override {
        std::lock_guard lock{m_mutex};
        if (get_connected() == &connected_to) {
            set_connection_internal(nullptr);
        }
    }


    void connect(Node<T>& node) {
        std::lock_guard lock{m_mutex};
        set_connection_internal(&node);
    }


    void disconnect() {
        std::lock_guard lock{m_mutex};
        set_connection_internal(nullptr);
    }


protected:
    virtual void set_connection_internal(Node<T>* node) {
        if (node) {
            m_node = node;
        } else {
            m_node = nullptr;
        }
    };


    Node<T>* get_node() const { return m_node; }


    void set_node(Node<T>* node) { m_node = node; }


private:
    std::mutex m_mutex;
    Node<T>* m_node = nullptr;
};


#endif //SERIALISTLOOPER_SOCKET_BASE_H
