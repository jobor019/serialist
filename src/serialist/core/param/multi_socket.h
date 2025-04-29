
#ifndef SERIALIST_MULTI_SOCKET_H
#define SERIALIST_MULTI_SOCKET_H

#include "core/param/socket_handler.h"
#include "types/index.h"

namespace serialist {
template<typename T>
class MultiSocket {
public:
    MultiSocket(Vec<Node<T>*> nodes, SocketHandler& socket_handler, const std::string& base_name)
        : m_sockets{create_sockets(std::move(nodes), socket_handler, base_name)} {
        assert(!m_sockets.empty());
    }


    static Vec<std::reference_wrapper<Socket<T>>> create_sockets(Vec<Node<T>*> nodes
                                                                 , SocketHandler& socket_handler
                                                                 , const std::string& base_name) {
        auto sockets = Vec<std::reference_wrapper<Socket<T>>>::allocated(nodes.size());
        for (std::size_t i = 0; i < nodes.size(); ++i) {
            sockets.append(std::ref(socket_handler.create_socket(base_name + std::to_string(i), nodes[i])));
        }

        return sockets;
    }


    Vec<Voices<T>> process() {
        auto result = Vec<Voices<T>>::allocated(m_sockets.size());
        for (const auto& socket : m_sockets) {
            result.append(socket.get().process());
        }
        return result;
    }


    bool any_is_connected() const {
        for (const auto& socket : m_sockets) {
            if (socket.get().is_connected()) {
                return true;
            }
        }
        return false;
    }


    std::size_t size() const { return m_sockets.size(); }

private:
    Vec<std::reference_wrapper<Socket<T>>> m_sockets;

};

}


#endif //SERIALIST_MULTI_SOCKET_H
