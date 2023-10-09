
#ifndef SERIALISTLOOPER_CLASSIFIERS_H
#define SERIALISTLOOPER_CLASSIFIERS_H

#include "core/algo/collections/vec.h"

template<typename InputType>
class LinearBandClassifier {
public:
    LinearBandClassifier(InputType min, InputType max, InputType band_width)
            : m_min(min)
              , m_max(max)
              , m_band_width(band_width)
              , m_num_classes(compute_n_bands(min, max, band_width)) {
        static_assert(std::is_arithmetic_v<InputType>, "InputType must be an arithmetic type");
    }

    std::size_t classify(const InputType& value) const {
        if (value < m_min)
            return 0;
        if (value >= m_max) {
            return m_num_classes - 1;
        }

        return static_cast<std::size_t>((value - m_min) / m_band_width);
    }

    Vec<std::size_t> classify(const Vec<InputType>& values) const {
        Vec<std::size_t> output(values.size());
        for (std::size_t i = 0; i < values.size(); ++i) {
            output[i] = classify(values[i]);
        }
        return output;
    }

    std::size_t get_num_classes() const {
        return m_num_classes;
    }

    InputType start_of(std::size_t band) const {
        if (band >= m_num_classes)
            band = m_num_classes - 1;

        return m_min + static_cast<InputType>(band) * m_band_width;
    }

    InputType end_of(std::size_t band) const {
        if (band >= m_num_classes - 1)
            return m_max;

        return m_min + static_cast<InputType>(band + 1) * m_band_width;
    }

    std::pair<InputType, InputType> bounds_of(std::size_t band) const {
        return std::make_pair(start_of(band), end_of(band));
    }


    void set_min(InputType min) {
        m_min = min;
        m_num_classes = compute_n_bands(m_min, m_max, m_band_width);
    }


    void set_max(InputType max) {
        m_max = max;
        m_num_classes = compute_n_bands(m_min, m_max, m_band_width);
    }


    void set_band_width(InputType band_width) {
        m_band_width = band_width;
        m_num_classes = compute_n_bands(m_min, m_max, m_band_width);
    }


private:

    static std::size_t compute_n_bands(const InputType& min, const InputType& max, const InputType& band_width) {
        if constexpr (std::is_integral_v<InputType>) {
            return static_cast<std::size_t>(std::ceil(static_cast<double>(max - min) / static_cast<double>(band_width)));
        }
        return static_cast<std::size_t>((max - min) / band_width);
    }


    InputType m_min;
    InputType m_max;
    InputType m_band_width;

    std::size_t m_num_classes;
};


#endif //SERIALISTLOOPER_CLASSIFIERS_H
