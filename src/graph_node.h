

#ifndef SERIALIST_LOOPER_GRAPH_NODE_H
#define SERIALIST_LOOPER_GRAPH_NODE_H

#include "transport.h"
#include "mapping.h"

template<typename T>
class GraphNode {
public:

    GraphNode() = default;

    virtual ~GraphNode() = default;

    GraphNode(const GraphNode&) = default;

    GraphNode& operator=(const GraphNode&) = default;

    GraphNode(GraphNode&&)  noexcept = default;

    GraphNode& operator=(GraphNode&&)  noexcept = default;


    virtual std::vector<T> process(const TimePoint& time) = 0;
};

#endif //SERIALIST_LOOPER_GRAPH_NODE_H
