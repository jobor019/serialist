
#ifndef SERIALIST_GUI_SOCKET_POLICY_H
#define SERIALIST_GUI_SOCKET_POLICY_H

#include "gui/param/vt_socket.h"

namespace serialist {


#ifdef SERIALIST_OVERRIDE_POLICIES


template<typename T>
using Socket = VTSocket<T>;


#endif // SERIALIST_OVERRIDE_POLICIES


} // namespace serialist

#endif //SERIALIST_GUI_SOCKET_POLICY_H


