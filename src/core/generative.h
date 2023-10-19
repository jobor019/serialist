

#ifndef SERIALIST_LOOPER_GENERATIVE_H
#define SERIALIST_LOOPER_GENERATIVE_H

#include "core/algo/time/transport.h"
#include "core/param/parameter_policy.h"
#include "core/collections/voices.h"

#include <optional>

class Generative {
public:
    Generative() = default;


    virtual ~Generative() = default;
    Generative(const Generative&) = delete;
    Generative& operator=(const Generative&) = delete;
    Generative(Generative&&) noexcept = delete;
    Generative& operator=(Generative&&) noexcept = delete;

    virtual std::vector<Generative*> get_connected() = 0;
    virtual ParameterHandler& get_parameter_handler() = 0;
    virtual void disconnect_if(Generative&) {}


    virtual void update_time(const TimePoint&) {}
};


// ==============================================================================================

class Root : public Generative {
public:
    virtual void process() = 0;
};


// ==============================================================================================

template<typename T>
class Node : public Generative {
public:
    virtual Voices<T> process() = 0;
};


#endif //SERIALIST_LOOPER_GENERATIVE_H
