
#ifndef SERIALISTLOOPER_CLASSIFIERS_H
#define SERIALISTLOOPER_CLASSIFIERS_H

#include "core/collections/vec.h"

template<typename T>
class LinearBandClassifier {
public:
    LinearBandClassifier(T min, T max, std::size_t num_classes)
            : m_min(min)
              , m_max(max)
              , m_num_classes(num_classes) {
        static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type");
        update_band_width();
    }


    std::size_t classify(const T& value) const {
        if (value < m_min)
            return 0;
        if (value >= m_max) {
            return m_num_classes - 1;
        }

        return class_of(value);
    }


    Vec<std::size_t> classify(const Vec<T>& values) const {
        auto output = Vec<std::size_t>::allocated(values.size());
        for (const auto& v: values) {
            output.append(classify(v));
        }
        return output;
    }


    std::size_t get_num_classes() const {
        return m_num_classes;
    }


    T start_of(std::size_t cls) const {
        if (cls >= m_num_classes)
            cls = m_num_classes - 1;

        return lower_bound_of(cls);
    }


    T end_of(std::size_t cls) const {
        if (cls >= m_num_classes - 1)
            return m_max;

        return lower_bound_of(cls + 1);
    }


    std::pair<T, T> bounds_of(std::size_t band) const {
        return std::make_pair(start_of(band), end_of(band));
    }


    void set_min(T min) {
        m_min = min;
        update_band_width();
    }


    void set_max(T max) {
        m_max = max;
        update_band_width();
    }


    void set_num_classes(std::size_t num_classes) {
        m_num_classes = num_classes;
        update_band_width();
    }

    Vec<T> bands() const {
        return Vec<std::size_t>::range(m_num_classes).template as_type<T>([this](const auto& v) {return start_of(v);});
    }


private:
    T lower_bound_of(std::size_t cls) const {
        if constexpr (std::is_integral_v<T>) {
            return m_min + static_cast<T>(std::floor(static_cast<double>(cls) * m_band_width));
        } else {
            return m_min + static_cast<T>(static_cast<double>(cls) * m_band_width);
        }
    }


    std::size_t class_of(const T& value) const {
        return static_cast<std::size_t>(std::floor(static_cast<double>(value - m_min) / m_band_width));
    }


    void update_band_width() {
        if constexpr (std::is_integral_v<T>) {
            m_band_width = static_cast<double>(m_max - m_min + 1) / static_cast<double>(m_num_classes);
        } else {
            m_band_width = static_cast<double>(m_max - m_min) / static_cast<double>(m_num_classes);
        }
    }


    T m_min;
    T m_max;
    std::size_t m_num_classes;

    double m_band_width = 0.0;
};


#endif //SERIALISTLOOPER_CLASSIFIERS_H
