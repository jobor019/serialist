
#ifndef SERIALISTLOOPER_STATE_IDS_H
#define SERIALISTLOOPER_STATE_IDS_H

namespace ModuleIds {

const int Null = 0;
const int ConfigurationLayerComponent = 1;
const int MappingComponent = 2;
const int TransportComponent = 3;

} // namespace ModuleIds


// ==============================================================================================

namespace StateIds {

const int NoInteraction = -1;
const int Default = 0;

namespace Configuration {
const int Move = 1;
}

} // namespace StateIds


#endif //SERIALISTLOOPER_STATE_IDS_H
