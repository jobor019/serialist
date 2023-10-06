
#ifndef SERIALISTLOOPER_RANDOM_H
#define SERIALISTLOOPER_RANDOM_H

#include <random>
#include "core/algo/collections/vec.h"

class Random {
public:
    explicit Random(std::optional<unsigned int> seed = std::nullopt) : m_rng(seed.value_or(std::random_device()())) {}


    double next() {
        return m_distribution(m_rng);
    }


    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    T next(T lower_bound, T upper_bound) {
        return static_cast<T>(lower_bound + m_distribution(m_rng) * (upper_bound - lower_bound));
    }


    /**
     * @throw std::out_of_range if `values` is empty
     */
    template<typename T>
    const T& choice(const Vec<T>& values) {
        return values[static_cast<std::size_t>(std::floor(m_distribution(m_rng) * static_cast<double>(values.size())))];
    }


    template<typename T>
    Vec<T> scramble(const Vec<T>& values) {
        Vec<T> scrambled = values;
        std::shuffle(scrambled.begin(), scrambled.end(), m_rng);
        return scrambled;
    }


    /**
     * @throw std::out_of_range if `values` is empty
     */
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    std::size_t weighted_choice(const Vec<T>& values) {
        if (values.empty()) {
            throw std::out_of_range("Cannot choose from an empty vector");
        }

        auto cdf = values.cloned().cumsum();


        auto choice = static_cast<T>(next() * cdf.back());

        auto it = std::lower_bound(cdf.begin(), cdf.end(), choice);
        if (it == cdf.end()) {
            return cdf.size() - 1;
        }
        return static_cast<std::size_t>(it - cdf.begin());
    }


private:
    std::mt19937 m_rng;
    std::uniform_real_distribution<> m_distribution{0.0, 1.0};
};


// ==============================================================================================




#endif //SERIALISTLOOPER_RANDOM_H
