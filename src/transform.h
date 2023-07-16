
#ifndef SERIALISTLOOPER_TRANSFORM_H
#define SERIALISTLOOPER_TRANSFORM_H

#include "generative.h"
#include "socket_policy.h"

template<typename T>
class Transform : public Node<T> {
    // TODO: IS A Node but HAS A Socket? Should support all `connect/disconnect` operations,
    //  but can do it through composition rather than inheritance, I think.

};

#endif //SERIALISTLOOPER_TRANSFORM_H
