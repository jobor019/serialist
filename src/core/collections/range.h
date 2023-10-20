
#ifndef SERIALISTLOOPER_SPACES_H
#define SERIALISTLOOPER_RANGE_H

#include <type_traits>
#include <cassert>

template<typename T>
class ArithmeticRange {
public:
    ArithmeticRange(T start, T end) : m_start(start), m_end(end) {
        static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type");
        assert(m_start <= m_end);
    }


    bool is_in(T value) const {
        return value >= m_start && value < m_end;
    }


private:
    double m_start;
    double m_end;
};

#endif //SERIALISTLOOPER_SPACES_H
