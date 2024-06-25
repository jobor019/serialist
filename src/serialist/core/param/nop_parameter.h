

#ifndef SERIALISTPLAYGROUND_NOP_PARAMETER_H
#define SERIALISTPLAYGROUND_NOP_PARAMETER_H


#include <iostream>
#include "core/exceptions.h"
#include "core/collections/voices.h"

class NopParameterHandler {
public:
    // Public ctor, signature to match VTParameterHandler
    explicit NopParameterHandler(const std::string&, NopParameterHandler&, const std::string& = "") {}


    NopParameterHandler() = default;


    template<typename T>
    void add_static_property(const std::string&, T) { /* unused*/} // signature to match VTParameterHandler


};


// ==============================================================================================

template<typename T>
class NopParameter {
public:
    // Signature to match VTParameter
    NopParameter(T value, const std::string&, NopParameterHandler&) : m_value(value) {}


    T get() const { return m_value; }


    void set(const T& value) { m_value = value; }


private:
    T m_value;

};


// ==============================================================================================

template<typename OutputType, typename StoredType = OutputType>
class NopSequenceParameter {
public:

    // Signature to match VTParameter
    NopSequenceParameter(const std::string&, NopParameterHandler&, const Voices<StoredType>& initial)
            : m_voices(initial.template as_type<OutputType>()) {}


    void set(const Voices<StoredType>& v) {
        m_voices = v.template as_type<OutputType>();
    }


    const Voices<OutputType>& get_voices() {
        return m_voices;
    }


private:

    Voices<OutputType> m_voices;

};

#endif //SERIALISTPLAYGROUND_NOP_PARAMETER_H
