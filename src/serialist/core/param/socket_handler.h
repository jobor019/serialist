
#ifndef SERIALISTLOOPER_SOCKET_HANDLER_H
#define SERIALISTLOOPER_SOCKET_HANDLER_H

#include <memory>
#include "core/connectable.h"
#include "core/param/parameter_keys.h"

namespace serialist {

class SocketHandler {
public:
    explicit SocketHandler(ParameterHandler& parameter_handler)
            : m_socket_parameter_handler(Specification(param::types::sockets_tree), parameter_handler) {}


    template<typename T>
    Socket<T>& create_socket(const std::string& id, Node<T>* initial = nullptr) {
        m_sockets.emplace_back(std::make_unique<Socket<T>>(id, m_socket_parameter_handler, initial));
        return dynamic_cast<Socket<T>&>(*m_sockets.back());
    }


    std::vector<Generative*> get_connected() const {
        std::vector<Generative*> generatives;

        for (auto& socket: m_sockets) {
            if (auto* generative = socket->get_connected()) {
                generatives.push_back(generative);
            }
        }
        return generatives;
    }


    void disconnect_if(Generative& connected_to) {
        for (const auto& socket: m_sockets) {
            socket->disconnect_if(connected_to);
        }
    }


//    std::vector<std::unique_ptr<Connectable>> get_sockets() {
//        return m_sockets;
//    }


private:
    ParameterHandler m_socket_parameter_handler;

    std::vector<std::unique_ptr<Connectable>> m_sockets;
};

} // namespace serialist

#endif //SERIALISTLOOPER_SOCKET_HANDLER_H
