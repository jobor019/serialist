

#ifndef SERIALISTLOOPER_SEQUENCE_H
#define SERIALISTLOOPER_SEQUENCE_H

#include "parameter_policy.h"
#include "generative.h"

template<typename T>
class Sequence : public Node<T>
        , public ParameterHandler {
    explicit Sequence(std::vector<T> value, const std::string& id, VTParameterHandler& parent)
    : ParameterHandler(id, parent), m_value(value, m_parameter_address, *this) {}

};


#endif //SERIALISTLOOPER_SEQUENCE_H
