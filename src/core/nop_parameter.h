

#ifndef SERIALISTPLAYGROUND_NOP_PARAMETER_H
#define SERIALISTPLAYGROUND_NOP_PARAMETER_H


#include <iostream>
#include "exceptions.h"
#include "interpolator.h"

class NopParameterHandler {
public:
    // Public ctor, same template as VTParameterHandler
    explicit NopParameterHandler(const std::string&, NopParameterHandler&, const std::string& = "") {}

    NopParameterHandler() = default;

    template<typename T>
    void add_static_property(const std::string&, T) {}


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

template<typename T>
class NopParametrizedSequence {
public:

    NopParametrizedSequence(const std::string&, NopParameterHandler&, const std::vector<T>& initial) {
        for (auto& v: initial) {
            insert(v, -1);
        }
    }


    T at(int index) {
        index = adjust_index_range(index, false);
        return m_values.at(static_cast<std::size_t>(index));
    }


    std::vector<T> clone_values() {
        return std::vector<T>(m_values);
    }


    void reset_values(std::vector<T> new_values) {
        m_values.clear();
        for (auto& value: new_values) {
            internal_insert(value, -1);
        }
    }


    std::vector<T> interpolate(double position, const InterpolationStrategy& strategy) {
        return Interpolator<T>::interpolate(position, strategy, m_values);
    }


    void insert(T value, int index) {
        internal_insert(value, index);

    }


    void move(int index_from, int index_to) {
        index_from = adjust_index_range(index_from, false);
        index_to = adjust_index_range(index_to, true);

        std::rotate(m_values.begin() + index_from, m_values.begin() + index_from + 1, m_values.begin() + index_to);

    }


    void remove(int index) {
        index = adjust_index_range(index, false);
        m_values.erase(m_values.begin() + index);
    }


    const std::size_t& size() {
        return m_values.size();
    }


    bool empty() {
        return m_values.empty();
    }


private:

    int adjust_index_range(int index, bool for_insertion) {

        // negative indices: insert/access from back
        if (index < 0)
            index += static_cast<int>(m_values.size()) + static_cast<int>(for_insertion);

        return std::clamp(index, 0, static_cast<int>(m_values.size()) - static_cast<int>(!for_insertion));
    }


    void internal_insert(const T& value, int index) {
        index = adjust_index_range(index, true);
        m_values.insert(m_values.begin() + index, value);
    }


    std::vector<T> m_values;

};

#endif //SERIALISTPLAYGROUND_NOP_PARAMETER_H
