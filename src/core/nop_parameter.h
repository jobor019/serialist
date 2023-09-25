

#ifndef SERIALISTPLAYGROUND_NOP_PARAMETER_H
#define SERIALISTPLAYGROUND_NOP_PARAMETER_H


#include <iostream>
#include "exceptions.h"
#include "interpolator.h"
#include "voice.h"

class NopParameterHandler {
public:
    // Public ctor, same template as VTParameterHandler
    explicit NopParameterHandler(const std::string&, NopParameterHandler&, const std::string& = "") {}


    NopParameterHandler() = default;


    template<typename T>
    void add_static_property(const std::string&, T) { /* unused*/}


};


// ==============================================================================================

template<typename T>
class NopParameter {
public:
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

    NopSequenceParameter(const std::string&, NopParameterHandler&
                         , const std::vector<std::vector<StoredType>>& initial) {
        set(initial);
    }


    void set(const std::vector<std::vector<StoredType>>& v) {
        m_voices = Voices<OutputType>(v);
    }


    void set_transposed(const std::vector<StoredType>& v) {
        auto v_transposed = VoiceUtils::transpose(v);
        m_voices = Voices<OutputType>(v_transposed);
    }


    const Voices<OutputType>& get_voices() {
        return m_voices;
    }


    const std::vector<std::vector<StoredType>>& get_values() {
        m_voices.vectors_as();
    }


private:

    Voices<OutputType> m_voices = Voices<OutputType>::create_empty_like();

};

#endif //SERIALISTPLAYGROUND_NOP_PARAMETER_H
