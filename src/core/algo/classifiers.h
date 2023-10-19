
#ifndef SERIALISTLOOPER_CLASSIFIERS_H
#define SERIALISTLOOPER_CLASSIFIERS_H

#include "core/collections/vec.h"

template<typename InputType>
class LinearBandClassifier {
public:
    LinearBandClassifier(InputType min, InputType max, std::size_t num_classes)
            : m_min(min)
              , m_max(max)
              , m_num_classes(num_classes) {
        static_assert(std::is_arithmetic_v<InputType>, "InputType must be an arithmetic type");
        update_band_width();
    }


    std::size_t classify(const InputType& value) const {
        if (value < m_min)
            return 0;
        if (value >= m_max) {
            return m_num_classes - 1;
        }

        return class_of(value);
    }


    Vec<std::size_t> classify(const Vec<InputType>& values) const {
        auto output = Vec<std::size_t>::allocated(values.size());
        for (const auto& v: values) {
            output.append(classify(v));
        }
        return output;
    }


    std::size_t get_num_classes() const {
        return m_num_classes;
    }


    InputType start_of(std::size_t cls) const {
        if (cls >= m_num_classes)
            cls = m_num_classes - 1;

        return lower_bound_of(cls);
    }


    InputType end_of(std::size_t cls) const {
        if (cls >= m_num_classes - 1)
            return m_max;

        return lower_bound_of(cls + 1);
    }


    std::pair<InputType, InputType> bounds_of(std::size_t band) const {
        return std::make_pair(start_of(band), end_of(band));
    }


    void set_min(InputType min) {
        m_min = min;
        update_band_width();
    }


    void set_max(InputType max) {
        m_max = max;
        update_band_width();
    }


    void set_num_classes(std::size_t num_classes) {
        m_num_classes = num_classes;
        update_band_width();
    }


private:
    InputType lower_bound_of(std::size_t cls) const {
        if constexpr (std::is_integral_v<InputType>) {
            return m_min + static_cast<InputType>(std::floor(static_cast<double>(cls) * m_band_width));
        } else {
            return m_min + static_cast<InputType>(static_cast<double>(cls) * m_band_width);
        }
    }


    std::size_t class_of(const InputType& value) const {
        return static_cast<std::size_t>(std::floor(static_cast<double>(value - m_min) / m_band_width));
    }


    void update_band_width() {
        if constexpr (std::is_integral_v<InputType>) {
            m_band_width = static_cast<double>(m_max - m_min + 1) / static_cast<double>(m_num_classes);
        } else {
            m_band_width = static_cast<double>(m_max - m_min) / static_cast<double>(m_num_classes);
        }
    }


    InputType m_min;
    InputType m_max;
    std::size_t m_num_classes;

    double m_band_width = 0.0;
};


#endif //SERIALISTLOOPER_CLASSIFIERS_H
