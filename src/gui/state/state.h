
#ifndef SERIALISTLOOPER_STATE_H
#define SERIALISTLOOPER_STATE_H

namespace StateIdentifiers {

const unsigned int ConfigurationLayerComponent = 1;
const unsigned int MappingComponent = 2;
const unsigned int TransportComponent = 3;

} // namespace StateIdentifiers


// ==============================================================================================


struct State {
    unsigned int module_id;
    unsigned int state_id;

    static State null_state() {
        return State{0, 0};
    }

    bool operator==(const State& other) const {
        return module_id == other.module_id && state_id == other.state_id;
    }

    bool operator!=(const State& other) const {
        return !(*this == other);
    }

    bool is_null() const {
        return module_id == 0 && state_id == 0;
    }
};

#endif //SERIALISTLOOPER_STATE_H
