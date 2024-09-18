
#ifndef SERIALIST_PARAMETER_POLICY_H
#define SERIALIST_PARAMETER_POLICY_H

#include "core/param/nop_parameter.h"

namespace serialist {

/**
 * See "core/policies/policies.h" for details
 */

#ifndef SERIALIST_OVERRIDE_POLICIES


using ParameterHandler = NopParameterHandler;

template<typename DataType>
using AtomicParameter = NopParameter<DataType>;

template<typename DataType>
using ComplexParameter = NopParameter<DataType>;


template<typename OutputType, typename StoredType = OutputType>
using SequenceParameter = NopSequenceParameter<OutputType, StoredType>;


#endif //SERIALIST_OVERRIDE_POLICIES

} // namespace serialist


#endif //SERIALIST_PARAMETER_POLICY_H
