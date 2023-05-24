

#ifndef SERIALISTLOOPER_SEQUENCE_H
#define SERIALISTLOOPER_SEQUENCE_H

#include "parameter_policy.h"
#include "generative.h"
#include "interpolator.h"

template<typename T>
class Sequence : public Generative
                 , public ParameterHandler {
public:

    inline static const std::string PARAMETER_ADDRESS = "sequence";


    explicit Sequence(const std::string& id, ParameterHandler& parent, const std::vector<T>& initial_values = {})
            : ParameterHandler(id, parent), m_sequence(initial_values, PARAMETER_ADDRESS, *this) {}


    std::vector<T> process(const TimePoint&, double y, const InterpolationStrategy<T> strategy) {
        return m_sequence.interpolate(y, strategy);
    }


    ParametrizedSequence<T>& get_parameter_obj() {
        return m_sequence;
    }


    std::vector<Generative*> get_connected() override {
        return {};
    }


private:
    ParametrizedSequence<T> m_sequence;

};


#endif //SERIALISTLOOPER_SEQUENCE_H
