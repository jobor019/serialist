

#ifndef SERIALIST_LOOPER_GRAPH_NODE_H
#define SERIALIST_LOOPER_GRAPH_NODE_H

#include "transport.h"
#include "mapping.h"

#include <optional>

template<typename T>
class GraphNode {
public:

    GraphNode() = default;

    virtual ~GraphNode() = default;

    GraphNode(const GraphNode&) = default;

    GraphNode& operator=(const GraphNode&) = default;

    GraphNode(GraphNode&&) noexcept = default;

    GraphNode& operator=(GraphNode&&) noexcept = default;


    virtual std::vector<T> process(const TimePoint& time) = 0;
};


// ==============================================================================================

template<typename T>
class Parameter : public GraphNode<T> {
public:
    explicit Parameter(T value) : m_value(value) {}


    std::vector<T> process(const TimePoint& time) override {
        return {m_value};
    }


    [[nodiscard]]
    T get_value() const {
        return m_value;
    }


    void set_value(T value) {
        m_value = value;
    }


private:
    T m_value;

};


#endif //SERIALIST_LOOPER_GRAPH_NODE_H
