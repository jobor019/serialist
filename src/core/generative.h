

#ifndef SERIALIST_LOOPER_GENERATIVE_H
#define SERIALIST_LOOPER_GENERATIVE_H

#include "transport.h"
#include "parameter_policy.h"
#include "voice.h"

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
    virtual void disconnect_if(Generative& connected_to) = 0;


    virtual void update_time(const TimePoint&) {}


    template<std::size_t max_count = 128, typename... Args>
    static std::size_t compute_voice_count(const Voices<Facet>& voices, Args... args) {
        auto num_voices = static_cast<long>(voices.adapted_to(1).front_or(0));
        if (num_voices <= 0) {
            return std::min(max_count, std::max({static_cast<std::size_t>(1), args...}));
        }

        return std::min(max_count, static_cast<std::size_t>(num_voices));
    }

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
