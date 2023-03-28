

#ifndef SERIALIST_LOOPER_LOOPER_H
#define SERIALIST_LOOPER_LOOPER_H

#include "phasor.h"
#include "map.h"
#include "graph_node.h"

template<typename T>
class Looper : public GraphNode<T> {
public:
    explicit Looper() = default;

    explicit Looper(Phasor phasor) : phasor(phasor) {}

//    explicit Looper(Phasor phasor, Mapping mapping) {}


    T process(const TimePoint& time) override {
        return MapElement<T>();
    }


private:
    Phasor phasor;
    std::vector<MapElement<T> > mapping;  // TODO: Tempo: Should be a Mapping<MapEntry<T>> later

};

#endif //SERIALIST_LOOPER_LOOPER_H
