
#ifndef SERIALIST_POLICIES_H
#define SERIALIST_POLICIES_H


#include "core/param/nop_socket.h"

namespace serialist {

/**
 * All of the following define strategy patterns where dynamic polymorphism is unsuitable for one
 *     (or multiple) of the following reasons:
 *
 * - The strategy should be set once for the entire project
 * - The strategy depends on sets of multiple co-dependent classes (e.g. Parameter and ParameterHandler), where
 *      different types cannot be combined
 * - The strategy has one or multiple templated functions that would be virtual (e.g. `virtual T get()`)
 *
 * The default strategies will have no side-effects, but may be overridden to define side effects on
 *     setting parameters, sockets and/or other means of loading/storing state.
 */

#ifndef SERIALIST_OVERRIDE_POLICIES


using ParameterHandler = NopParameterHandler;

template<typename DataType>
using AtomicParameter = NopParameter<DataType>;

template<typename DataType>
using ComplexParameter = NopParameter<DataType>;


template<typename OutputType, typename StoredType = OutputType>
using SequenceParameter = NopSequenceParameter<OutputType, StoredType>;


// ==============================================================================================

template<typename T>
using Socket = NopSocket<T>;


// ==============================================================================================

// TODO: DeserializationData


#endif //SERIALIST_OVERRIDE_POLICIES

} // namespace serialist

#endif //SERIALIST_POLICIES_H
