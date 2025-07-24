
#ifndef SERIALISTLOOPER_SOCKET_BASE_H
#define SERIALISTLOOPER_SOCKET_BASE_H

#include <mutex>
#include "core/connectable.h"
#include "core/generative.h"
#include "core/param/parameter_keys.h"

namespace serialist {

template<typename T>
class SocketBase : public Connectable {
public:
    static Specification specification(std::string member_name) {
        return Specification(param::types::socket)
                .with_name_in_parent(std::move(member_name));
    }

    explicit SocketBase(Node<T>* initial = nullptr)
            : m_node(initial) {}


    Voices<T> process() {
        std::lock_guard lock{m_mutex};
        auto v = process_internal();
        m_previous_value = v;
        return v;
    }


    Voices<T> process(std::size_t num_voices) {
        return process().adapted_to(num_voices);
    }


    // TODO: This seems incredibly inefficient?
    std::optional<Voices<T>> process_if_changed() {
        std::lock_guard lock{m_mutex};
        return has_changed_internal() ? std::make_optional(process_internal()) : std::nullopt;
    }


    bool has_changed() {
        std::lock_guard lock{m_mutex};
        return has_changed_internal();
    }

    std::size_t voice_count() {
        std::lock_guard lock{m_mutex};
        return process_internal().size();
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
    Voices<T> process_internal() {
        if (m_node == nullptr)
            return Voices<T>::empty_like();
        return m_node->process();
    }


    bool has_changed_internal() {
        // Note: calling Node.process() multiple times in the same time step won't change the node's value
        return !m_previous_value.has_value() || m_previous_value != process_internal();
    }


    virtual void set_connection_internal(Node<T>* node) {
        if (node) {
            m_node = node;
        } else {
            m_node = nullptr;
        }
        m_previous_value = std::nullopt;
    }


    Node<T>* get_node() const { return m_node; }


    void set_node(Node<T>* node) { m_node = node; }


private:
    std::mutex m_mutex;
    Node<T>* m_node = nullptr;

    std::optional<Voices<T>> m_previous_value = std::nullopt;
};


} // namespace serialist

#endif //SERIALISTLOOPER_SOCKET_BASE_H
