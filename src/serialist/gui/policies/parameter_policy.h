
#ifndef SERIALIST_GUI_PARAMETER_POLICY_H
#define SERIALIST_GUI_PARAMETER_POLICY_H

#include "gui/param/vt_parameter.h"

namespace serialist {


#ifdef SERIALIST_OVERRIDE_POLICIES


using ParameterHandler = VTParameterHandler;

template<typename DataType>
using AtomicParameter = AtomicVTParameter<DataType>;

template<typename DataType>
using ComplexParameter = LockingVTParameter<DataType>;

template<typename PivotType, typename StoredType = PivotType>
using SequenceParameter = VTSequenceParameter<PivotType, StoredType>;


#endif // SERIALIST_OVERRIDE_POLICIES


} // namespace serialist

#endif //SERIALIST_GUI_PARAMETER_POLICY_H
