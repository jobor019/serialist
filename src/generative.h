

#ifndef SERIALIST_LOOPER_GENERATIVE_H
#define SERIALIST_LOOPER_GENERATIVE_H

#include "transport.h"
#include "parameter_policy.h"

#include <optional>

template<typename T>
class Generative {
public:

    Generative() = default;

    virtual ~Generative() = default;

    Generative(const Generative&) = default;

    Generative& operator=(const Generative&) = default;

    Generative(Generative&&) noexcept = default;

    Generative& operator=(Generative&&) noexcept = default;

    virtual std::vector<T> process(const TimePoint& time) = 0;
};

#endif //SERIALIST_LOOPER_GENERATIVE_H
