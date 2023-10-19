
#ifndef SERIALISTLOOPER_STAT_H
#define SERIALISTLOOPER_STAT_H

#include "core/collections/vec.h"

template<typename T>
class Histogram {
public:
    explicit Histogram(const Vec<T>& values, bool sort_bins = true) {
        for (const T& value: values) {
            auto it = std::find(m_bins.begin(), m_bins.end(), value);
            if (it != m_bins.end()) {
                auto index = static_cast<std::size_t>(std::distance(m_bins.begin(), it));
                m_counts[index]++;
            } else {
                m_bins.append(value);
                m_counts.append(1);
            }
        }

        // TODO: Constexpr this to check comparability of T
        if (sort_bins) {
            auto sort_indices = m_bins.argsort(true, true);
            m_counts.reorder(sort_indices);
        }
    }

    Histogram(const Vec<T>& values, const Vec<T>& bins) : m_bins(bins) {
        m_counts = Vec<std::size_t>::repeated(m_bins.size(), 0);

        for (const T& value: values) {
            auto it = std::find(m_bins.begin(), m_bins.end(), value);
            if (it != m_bins.end()) {
                auto index = static_cast<std::size_t>(std::distance(m_bins.begin(), it));
                m_counts[index]++;
            } else {
                throw std::runtime_error("Invalid bin");
            }
        }
    }

    Histogram(const Vec<T>& values, std::size_t lower_class_bound, std::size_t upper_class_bound) {
        (void) values, lower_class_bound, upper_class_bound;
        throw std::runtime_error("not implemented: Histogram()"); // TODO

    }

    std::pair<const Vec<T>& , const Vec<std::size_t>&> get() const {
        return std::make_pair(m_bins, m_counts);
    }


    const Vec<T>& get_bins() const {
        return m_bins;
    }


    const Vec<std::size_t>& get_counts() const {
        return m_counts;
    }

private:
    Vec<T> m_bins;
    Vec<std::size_t> m_counts;

};

#endif //SERIALISTLOOPER_STAT_H
