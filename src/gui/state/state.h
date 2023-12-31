
#ifndef SERIALISTLOOPER_STATE_H
#define SERIALISTLOOPER_STATE_H

#include "state_ids.h"

class State {
public:
    State(int module_id, int state_id) : m_module_id(module_id), m_state_id(state_id) {}


    bool operator==(const State& other) const {
        return m_module_id == other.m_module_id && m_state_id == other.m_state_id;
    }


    bool operator!=(const State& other) const {
        return !(*this == other);
    }


    bool equals( int mod,  int state) const {
        return m_module_id == mod && m_state_id == state;
    }


     int get_module_id() const { return m_module_id; }


     int get_state_id() const { return m_state_id; }


private:
     int m_module_id;
     int m_state_id;
};

#endif //SERIALISTLOOPER_STATE_H
