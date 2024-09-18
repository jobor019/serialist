

#ifndef SERIALIST_LOOPER_GENERATIVE_H
#define SERIALIST_LOOPER_GENERATIVE_H

#include "core/algo/temporal/transport.h"
#include "core/collections/voices.h"
#include "core/algo/temporal/time_point.h"
#include "param/parameter_keys.h"


#include <optional>


namespace serialist {

class Generative {
public:
    static Specification specification(std::string id, std::string template_class) {
        return Specification(param::types::generative)
                .with_identifier(std::move(id))
                .with_template_class(std::move(template_class));
    }


    Generative() = default;

    virtual ~Generative() = default;

    Generative(const Generative&) = delete;

    Generative& operator=(const Generative&) = delete;

    Generative(Generative&&) noexcept = default;

    Generative& operator=(Generative&&) noexcept = default;

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

} // namespace serialist

#endif //SERIALIST_LOOPER_GENERATIVE_H
