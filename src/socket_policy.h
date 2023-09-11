

#ifndef SERIALISTLOOPER_SOCKET_POLICY_H
#define SERIALISTLOOPER_SOCKET_POLICY_H



#ifdef USE_JUCE

#include "vt_socket.h"

template<typename T>
using Socket = VTSocket<T>;

template<typename T>
using DataSocket = VTDataSocket<T>;
#else

#include "nop_socket.h"

template<typename OutputType>
using Socket = NopSocket<OutputType>;

template<typename OutputType>
using DataSocket = NopDataSocket<OutputType>;

#endif


#endif //SERIALISTLOOPER_SOCKET_POLICY_H
