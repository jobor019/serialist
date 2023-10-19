
#ifndef SERIALISTLOOPER_RANDOM_H
#define SERIALISTLOOPER_RANDOM_H

#include <random>
#include "core/collections/vec.h"

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



    template<typename T>
    T next(const Vec<std::pair<T, T>>& ranges) {
        (void) ranges;
        throw std::runtime_error("Not implemented: next(const Vec<std::pair<T, T>>& ranges)"); // TODO: Implement
    }

    template <typename T, typename... Args>
    Vec<T> nexts(std::size_t count, Args... args) {
        auto result = Vec<T>::allocated(count);
        for (std::size_t i = 0; i < count; ++i) {
            result.append(next(args...));
        }
        return std::move(result);
    }


    /**
     * @throw std::invalid_argument if `values` is empty
     */
    template<typename T>
    const T& choice(const Vec<T>& values) {
        if (values.empty()) {
            throw std::invalid_argument("Cannot choose from empty values");
        }

        return values[static_cast<std::size_t>(std::floor(m_distribution(m_rng) * static_cast<double>(values.size())))];
    }


    template<typename T>
    Vec<T> scramble(const Vec<T>& values) {
        Vec<T> scrambled = values;
        std::shuffle(scrambled.begin(), scrambled.end(), m_rng);
        return scrambled;
    }


    /**
     * @throw std::invalid_argument if `weights` is empty
     */
    template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
    std::size_t weighted_choice(const Vec<T>& weights) {
        if (weights.empty()) {
            throw std::invalid_argument("Cannot choose from empty weights");
        }

        auto cdf = weights.cloned().clip({0}, std::nullopt).cumsum();

        auto choice = static_cast<T>(next() * cdf.back());

        auto it = std::lower_bound(cdf.begin(), cdf.end(), choice);
        if (it == cdf.end()) {
            return cdf.size() - 1;
        }
        return static_cast<std::size_t>(std::distance(cdf.begin(), it));
    }


    template<typename WeightsType, typename ValuesType, typename = std::enable_if_t<std::is_arithmetic_v<WeightsType>>>
    WeightsType weighted_choice(const Vec<WeightsType>& weights, const Vec<ValuesType>& values) {
        return values[weighted_choice(weights)];
    }


private:
    std::mt19937 m_rng;
    std::uniform_real_distribution<> m_distribution{0.0, 1.0};
};


// ==============================================================================================




#endif //SERIALISTLOOPER_RANDOM_H
