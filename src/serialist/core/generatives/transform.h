
#ifndef SERIALISTLOOPER_TRANSFORM_H
#define SERIALISTLOOPER_TRANSFORM_H

#include "core/generative.h"
#include "core/param/socket_policy.h"

namespace serialist {

template<typename T>
class Transform : public Node<T> {
    // TODO: IS A Node but HAS A Socket? Should support all `connect/disconnect` operations,
    //  but can do it through composition rather than inheritance, I think.

};

} // namespace serialist

#endif //SERIALISTLOOPER_TRANSFORM_H
