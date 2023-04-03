

#ifndef SERIALIST_LOOPER_GRAPH_NODE_H
#define SERIALIST_LOOPER_GRAPH_NODE_H

#include "transport.h"
#include "mapping.h"

template<typename T>
class GraphNode {
public:
    virtual ~GraphNode() = default;


    virtual std::vector<T> process(const TimePoint& time) = 0;
};

#endif //SERIALIST_LOOPER_GRAPH_NODE_H
