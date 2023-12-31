
#ifndef SERIALISTLOOPER_STATES_H
#define SERIALISTLOOPER_STATES_H

#include "state.h"

namespace States {

const static inline State NoInteraction = State{ModuleIds::Null, StateIds::NoInteraction};
const static inline State Default = State{ModuleIds::Null, StateIds::Default};

const static inline State Move = State{ModuleIds::ConfigurationLayerComponent, StateIds::Configuration::Move};

}

#endif //SERIALISTLOOPER_STATES_H
