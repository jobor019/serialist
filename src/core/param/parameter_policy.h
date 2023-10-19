

#ifndef SERIALISTPLAYGROUND_PARAMETER_POLICY_H
#define SERIALISTPLAYGROUND_PARAMETER_POLICY_H

#ifdef USE_JUCE

#include "core/param/vt_parameter.h"

using ParameterHandler = VTParameterHandler;

template<typename DataType>
using AtomicParameter = AtomicVTParameter<DataType>;

template<typename DataType>
using ComplexParameter = LockingVTParameter<DataType>;

template<typename OutputType, typename StoredType = OutputType>
using SequenceParameter = VTSequenceParameter<OutputType, StoredType>;


#else

#include "core/param/nop_parameter.h"

using ParameterHandler = NopParameterHandler;

template<typename DataType>
using AtomicParameter = NopParameter<DataType>;

template<typename DataType>
using ComplexParameter = NopParameter<DataType>;


template<typename OutputType, typename StoredType = OutputType>
using SequenceParameter = NopSequenceParameter<OutputType, StoredType>;


#endif


#endif //SERIALISTPLAYGROUND_PARAMETER_POLICY_H
