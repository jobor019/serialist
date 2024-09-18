
#ifndef SERIALISTLOOPER_HISTOGRAM_H
#define SERIALISTLOOPER_HISTOGRAM_H

#include "core/collections/vec.h"
#include "core/algo/classifiers.h"

namespace serialist {

template<typename T>
class Histogram {
public:
    template<typename E =  T, typename = std::enable_if_t<!std::is_floating_point_v<E>>>
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


    template<typename E = T, typename = std::enable_if_t<std::is_arithmetic_v<E>>>
    Histogram(const Vec<T>& values, T lower_bound, T upper_bound, std::size_t num_bins) {
        if (lower_bound >= upper_bound || num_bins <= 1) {
            throw std::invalid_argument("Invalid arguments for Histogram constructor");
        }

        auto classifier = LinearBandClassifier<T>(lower_bound, upper_bound, num_bins);
        m_bins = classifier.bands();
        m_counts = Vec<std::size_t>::zeros(m_bins.size());

        for (const auto& value: values) {
            auto index = classifier.classify(value);
            m_counts[index]++;
        }
    }


    template<typename E = T, typename = std::enable_if_t<!std::is_floating_point_v<E>>>
    static Histogram with_discrete_bins(const Vec<T>& values, const Vec<T>& bins) {
        auto counts = Vec<std::size_t>::zeros(bins.size());

        for (const T& value: values) {
            auto it = std::find(bins.begin(), bins.end(), value);
            if (it != bins.end()) {
                auto index = static_cast<std::size_t>(std::distance(bins.begin(), it));
                counts[index]++;
            } else {
                throw std::invalid_argument("Value outside of provided bins detected");
            }
        }

        return Histogram(bins, counts);
    }

    // TODO: Reimplement. Currently doesn't create ranges for integral values, just discrete bins
//    template<typename E = T, typename = std::enable_if_t<std::is_integral_v<E>>>
//    Histogram(const Vec<T>& values, const Vec<T>& bins, bool sort_bins = true)
//            : m_bins(bins)
//              , m_counts(Vec<std::size_t>::repeated(m_bins.size(), 0)) {
//
//        if (sort_bins)
//            m_bins.sort();
//
//        if constexpr (std::is_arithmetic_v<T>) {
//            for (const T& value: values) {
//                for (std::size_t bin_index = 0; bin_index < m_bins.size(); ++bin_index) {
//                    if (value >= m_bins[bin_index]) {
//                        m_counts[bin_index]++;
//                        break;
//                    }
//                }
//                throw std::invalid_argument("Value outside of provided bins detected");
//            }
//        } else { // integral or discrete (non-arithmetic) values
//            for (const T& value: values) {
//                auto it = std::find(m_bins.begin(), m_bins.end(), value);
//                if (it != m_bins.end()) {
//                    auto index = static_cast<std::size_t>(std::distance(m_bins.begin(), it));
//                    m_counts[index]++;
//                } else {
//                    throw std::invalid_argument("Value outside of provided bins detected");
//                }
//            }
//        }
//    }





    std::pair<const Vec<T>&, const Vec<std::size_t>&> get() const {
        return std::make_pair(m_bins, m_counts);
    }


    const Vec<T>& get_bins() const {
        return m_bins;
    }


    const Vec<std::size_t>& get_counts() const {
        return m_counts;
    }


private:

    Histogram(const Vec<T>& bins, const Vec<std::size_t>& counts) : m_bins(bins), m_counts(counts) {}


    Vec<T> m_bins;
    Vec<std::size_t> m_counts;

};

} // namespace serialist

#endif //SERIALISTLOOPER_HISTOGRAM_H
