

#ifndef SERIALISTPLAYGROUND_PARAMETER_POLICY_H
#define SERIALISTPLAYGROUND_PARAMETER_POLICY_H

#ifdef USE_JUCE

#include "vt_parameter.h"

using ParameterHandler = VTParameterHandler;

template<typename T>
using AtomicParameter = AtomicVTParameter<T>;

template<typename T>
using ComplexParameter = LockingVTParameter<T>;

template<typename T>
using ParametrizedCollection = VTParametrizedSequence<T>; // TODO: Temp, until VTParametrizedSequence is handling lists of lists

template<typename T>
using ParametrizedSequence = VTParametrizedSequence<T>;

#else

#include "nop_parameter.h"

using ParameterHandler = NopParameterHandler;

template<typename DataType>
using AtomicParameter = NopParameter<DataType>;

template<typename DataType>
using ComplexParameter = NopParameter<DataType>;

template<typename OutputType>
using ParametrizedCollection = NopParametrizedSequence<OutputType>; // TODO: Temp, until VTParametrizedSequence is handling lists of lists

template<typename OutputType>
using ParametrizedSequence = NopParametrizedSequence<OutputType>;


#endif


#endif //SERIALISTPLAYGROUND_PARAMETER_POLICY_H
