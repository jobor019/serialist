
#ifndef SERIALISTLOOPER_STAT_H
#define SERIALISTLOOPER_STAT_H

#include "core/algo/collections/vec.h"

template<typename T>
class Histogram {
public:
    explicit Histogram(const Vec<T>& values) {
        for (const T& value: values) {
            auto it = std::find(m_values.begin(), m_values.end(), value);
            if (it != m_values.end()) {
                auto index = static_cast<std::size_t>(std::distance(m_values.begin(), it));
                m_counts[index]++;
            } else {
                m_values.append(value);
                m_counts.append(1);
            }
        }
    }


    const Vec<T>& get_values() const {
        return m_values;
    }


    const Vec<std::size_t>& get_counts() const {
        return m_counts;
    }


private:
    Vec<T> m_values;
    Vec<std::size_t> m_counts;

};

#endif //SERIALISTLOOPER_STAT_H
