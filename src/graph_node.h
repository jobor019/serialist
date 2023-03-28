

#ifndef SERIALIST_LOOPER_GRAPH_NODE_H
#define SERIALIST_LOOPER_GRAPH_NODE_H

#include "transport.h"
#include "map.h"

template<typename T>
class GraphNode {
public:
    // TODO: Should return some sort of container, not just a raw T
//    virtual MapElement<T> process(const TimePoint& time) = 0;
    virtual T process(const TimePoint& time) = 0;
};

#endif //SERIALIST_LOOPER_GRAPH_NODE_H
