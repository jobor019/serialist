

#ifndef SERIALIST_LOOPER_GENERATIVE_H
#define SERIALIST_LOOPER_GENERATIVE_H

#include "transport.h"

#include <optional>

class Generative {
public:
    Generative() = default;
    virtual ~Generative() = default;
    Generative(const Generative&) = delete;
    Generative& operator=(const Generative&) = delete;
    Generative(Generative&&) noexcept = default;
    Generative& operator=(Generative&&) noexcept = default;

    virtual std::vector<Generative*> get_connected() = 0;

protected:
    // TODO
//    template<typename... Args>
//    std::vector<Generative*> get_connected_if(Args*... args) {
//
//    }

};


// ==============================================================================================

class Source : public Generative {
public:
    virtual void process(const TimePoint& t) = 0;
};


// ==============================================================================================

template<typename T>
class Node : public Generative {
public:
    virtual std::vector<T> process(const TimePoint& t) = 0;


    template<typename NodeType>
    static NodeType value_or(Node<NodeType>* node, NodeType default_value, const TimePoint& t) {
        if (!node)
            return default_value;

        auto values = node->process((t));

        if (values.empty())
            return default_value;
        else
            return values.at(0);
    }
};


#endif //SERIALIST_LOOPER_GENERATIVE_H
