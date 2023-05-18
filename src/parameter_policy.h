

#ifndef SERIALISTPLAYGROUND_PARAMETER_POLICY_H
#define SERIALISTPLAYGROUND_PARAMETER_POLICY_H

#ifdef USE_JUCE

#include "vt_parameter.h"

using ParameterHandler = VTParameterHandler;

using ParameterListener = VTParameterListener;

template<typename T>
using AtomicParameter = AtomicVTParameter<T>;

template<typename T>
using ComplexParameter = LockingVTParameter<T>;

template<typename T>
using ParametrizedSequence = VTParametrizedSequence<T>;

#else

#include "nop_parameter.h"

using ParameterHandler = NopParameterHandler;

using ParameterListener = NopParameterListener; // TODO

template<typename T>
using AtomicParameter = NopParameter<T>;

template<typename T>
using ComplexParameter = NopParameter<T>;




#endif


#endif //SERIALISTPLAYGROUND_PARAMETER_POLICY_H
