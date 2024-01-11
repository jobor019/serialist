
#ifndef SERIALISTLOOPER_INPUT_CONDITION_H
#define SERIALISTLOOPER_INPUT_CONDITION_H

#include "input_mode.h"
#include "core/collections/vec.h"

class InputCondition {
public:
    InputCondition() = default;
    virtual ~InputCondition() = default;
    InputCondition(const InputCondition&) = delete;
    InputCondition& operator=(const InputCondition&) = delete;
    InputCondition(InputCondition&&) noexcept = default;
    InputCondition& operator=(InputCondition&&) noexcept = default;

    virtual bool is_met() const = 0;


    template<typename T>
    bool is() const {
        std::cout << "input cond\n";
        return typeid(T) == typeid(*this);
    }
};


// ==============================================================================================

class AlwaysTrueCondition : public InputCondition {
public:
    bool is_met() const override {
        return true;
    }
};


// ==============================================================================================

class KeyCondition : public InputCondition {
public:
    explicit KeyCondition(int key_code) : m_key_code(key_code) {}

    bool is_met() const override {
        return GlobalKeyState::is_down_exclusive(m_key_code);
    }

private:
    int m_key_code;
};


// ==============================================================================================

class NoKeyCondition : public InputCondition {
public:
    bool is_met() const override {
        return !GlobalKeyState::has_held_keys();
    }
};


#endif //SERIALISTLOOPER_INPUT_CONDITION_H
