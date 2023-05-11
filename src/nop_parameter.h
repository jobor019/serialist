

#ifndef SERIALISTPLAYGROUND_NOP_PARAMETER_H
#define SERIALISTPLAYGROUND_NOP_PARAMETER_H


#include <iostream>
#include <juce_data_structures/juce_data_structures.h>
#include "exceptions.h"

class NopParameterHandler {
public:
    // Public ctor, same template as VTParameterHandler
    explicit NopParameterHandler(const std::string&, NopParameterHandler&) {}


    static NopParameterHandler create_root() {
        return {};
    }


private:
    NopParameterHandler() = default;

};


// ==============================================================================================

template<typename T>
class NopParameter : private juce::ValueTree::Listener {
public:
    NopParameter(T value, const std::string&, NopParameterHandler&) : m_value(value) {}


    T get() const { return m_value; }


    void set(const T& value) { m_value = value; }


private:
    T m_value;

};

#endif //SERIALISTPLAYGROUND_NOP_PARAMETER_H
