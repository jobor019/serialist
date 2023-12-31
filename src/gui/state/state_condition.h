
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

class AlwaysTrueCondition : public Condition {
public:
    bool is_met() const override {
        return true;
    }
};

// ==============================================================================================



class KeyCondition : public Condition {
public:
    explicit KeyCondition(int key_code) : m_key_code(key_code) {}

    bool is_met() const override {
        return GlobalKeyState::is_down_exclusive(m_key_code);
    }

private:
    int m_key_code;
};


// ==============================================================================================

class NoKeyCondition : public Condition {
public:
    bool is_met() const override {
        return !GlobalKeyState::has_held_keys();
    }
};

#endif //SERIALISTLOOPER_STATE_CONDITION_H
