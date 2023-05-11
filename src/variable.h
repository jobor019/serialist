

#ifndef SERIALIST_LOOPER_VARIABLE_H
#define SERIALIST_LOOPER_VARIABLE_H


#include "generative.h"

template<typename T>
class Variable : public Generative<T> {
public:
    explicit Variable(T value) : m_value(value) {}


    std::vector<T> process(const TimePoint& time) override {
        (void) time;
        return {m_value};
    }


    T get_value() const { return m_value; }


    void set_value(T value) { m_value = value; }


private:
    T m_value;

};

#endif //SERIALIST_LOOPER_VARIABLE_H
