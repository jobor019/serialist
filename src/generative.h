

#ifndef SERIALIST_LOOPER_GENERATIVE_H
#define SERIALIST_LOOPER_GENERATIVE_H

#include "transport.h"

#include <optional>

class Generative {
public:
    Generative() = default;
    virtual ~Generative() = default;
    Generative(const Generative&) = default;
    Generative& operator=(const Generative&) = default;
    Generative(Generative&&) noexcept = default;
    Generative& operator=(Generative&&) noexcept = default;

    virtual std::vector<Generative*> get_connected() = 0;

};


// ==============================================================================================

class Source : public Generative {
public:
    Source() = default;
    virtual ~Source() = default;
    Source(const Source&) = default;
    Source& operator=(const Source&) = default;
    Source(Source&&) noexcept = default;
    Source& operator=(Source&&) noexcept = default;


    virtual void process(const TimePoint& time) = 0;
};


// ==============================================================================================

template<typename T>
class Node : public Generative {
public:

    Node() = default;

    virtual ~Node() = default;

    Node(const Node&) = default;

    Node& operator=(const Node&) = default;

    Node(Node&&) noexcept = default;

    Node& operator=(Node&&) noexcept = default;

    virtual std::vector<T> process(const TimePoint& time) = 0;
};

#endif //SERIALIST_LOOPER_GENERATIVE_H
