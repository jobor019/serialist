

#ifndef SERIALIST_LOOPER_GENERATIVE_H
#define SERIALIST_LOOPER_GENERATIVE_H

#include "transport.h"
#include "parameter_policy.h"
#include "interpolator.h"

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

    template<typename... Args, std::enable_if_t<std::conjunction_v<std::is_base_of<Generative, Args>...>, int> = 0>
    std::vector<Generative*> collect_connected(Args* ... args) {
        std::vector<Generative*> connected_generatives;

        ([&] {
            if (auto* elem = dynamic_cast<Generative*>(args)) {
                connected_generatives.emplace_back(elem);
            }
        }(), ...);
        return connected_generatives;
    }

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
};


// ==============================================================================================


template<typename T>
class DataNode : public Generative {
public:
    virtual std::vector<T> process(const TimePoint&, double y, InterpolationStrategy<T> strategy) = 0;
};


#endif //SERIALIST_LOOPER_GENERATIVE_H
