
#ifndef SERIALISTLOOPER_STATE_CONDITION_H
#define SERIALISTLOOPER_STATE_CONDITION_H

#include "key_state.h"

class Condition {
public:
    Condition() = default;
    virtual ~Condition() = default;
    Condition(const Condition&) = delete;
    Condition& operator=(const Condition&) = delete;
    Condition(Condition&&)  noexcept = default;
    Condition& operator=(Condition&&)  noexcept = default;

    virtual bool is_met() const = 0;
};


// ==============================================================================================

class KeyCondition {
public:

    explicit KeyCondition(int key_code) : m_key_code(key_code) {}

    bool is_met() const {
        return GlobalKeyState::is_down_exclusive(m_key_code);
    }

private:
    int m_key_code;
};

#endif //SERIALISTLOOPER_STATE_CONDITION_H
