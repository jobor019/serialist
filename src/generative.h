

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
    const static inline std::size_t AUTO_VOICES = 0;

    virtual void update_time(const TimePoint& t) = 0;
    virtual Voices<T> process() = 0;


    static std::size_t get_voice_count(const Voices<Facet>& voices, std::size_t max_voice_count = 128) {
        if (voices.is_empty_like())
            return AUTO_VOICES;

        auto num_voices = static_cast<long>(voices.adapted_to(1).front_or(0));
        if (num_voices <= 0) {
            return AUTO_VOICES;
        }
        return std::min(max_voice_count, static_cast<std::size_t>(num_voices));
    }
};


// ==============================================================================================

/**
 * Interface for Generatives manipulating temporality (e.g. a delay)
 */
class Temporal : public Generative {

public:
    virtual void step() = 0;
};


// ==============================================================================================

class InterpolationStrategy;

template<typename T>
class DataNode : public Generative {
public:
    virtual std::vector<T> process(const TimePoint&, double y, InterpolationStrategy strategy) = 0;
};


#endif //SERIALIST_LOOPER_GENERATIVE_H
