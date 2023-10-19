

#ifndef SERIALISTLOOPER_NOP_SOCKET_H
#define SERIALISTLOOPER_NOP_SOCKET_H

#include <string>

#include "core/param/socket_base.h"

template<typename T>
class NopSocket : public SocketBase<T> {
public:

    // signature to match VTSocket
    NopSocket(const std::string&, ParameterHandler&, Node<T>* initial = nullptr)
            : SocketBase<T>(initial) {}


    NopSocket<T>& operator=(Node<T>* node) {
        if (node)
            SocketBase<T>::connect(*node);
        else
            SocketBase<T>::disconnect();
        return *this;
    }

};


#endif //SERIALISTLOOPER_NOP_SOCKET_H
