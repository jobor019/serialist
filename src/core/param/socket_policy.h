

#ifndef SERIALISTLOOPER_SOCKET_POLICY_H
#define SERIALISTLOOPER_SOCKET_POLICY_H



#ifdef USE_JUCE

#include "core/param/vt_socket.h"

template<typename T>
using Socket = VTSocket<T>;

#else

#include "nop_socket.h"

template<typename T>
using Socket = NopSocket<T>;


#endif


#endif //SERIALISTLOOPER_SOCKET_POLICY_H
