
#ifndef SERIALIST_SOCKET_POLICY_H
#define SERIALIST_SOCKET_POLICY_H

#include "core/param/nop_socket.h"

/**
 * See "core/policies/policies.h" for details
 */


namespace serialist {


#ifndef SERIALIST_OVERRIDE_POLICIES


template<typename T>
using Socket = NopSocket<T>;


#endif //SERIALIST_OVERRIDE_POLICIES

} // namespace serialist

#endif //SERIALIST_SOCKET_POLICY_H
