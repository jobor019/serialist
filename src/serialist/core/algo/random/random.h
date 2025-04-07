#ifndef SERIALISTLOOPER_RANDOM_H
#define SERIALISTLOOPER_RANDOM_H

#include <random>
#include "core/collections/vec.h"


namespace serialist {
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


    template<typename T, typename... Args>
    Vec<T> nexts(std::size_t count, Args... args) {
        auto result = Vec<T>::allocated(count);
        for (std::size_t i = 0; i < count; ++i) {
            result.append(next(args...));
        }
        return std::move(result);
    }


    /**
     * Choose a single unweighted random element from `values`.
     *
     * @throw std::invalid_argument if `values` is empty
     */
    template<typename T>
    const T& choice(const Vec<T>& values) {
        if (values.empty()) {
            throw std::invalid_argument("Cannot choose from empty values");
        }

        return values[static_cast<std::size_t>(std::floor(m_distribution(m_rng) * static_cast<double>(values.size())))];
    }


    /**
     * Choose n unweighted random elements without duplicates from `values`.
     *
     * @param cycle_sampling If true, allow `num_choices` bigger than `values.size()`.
     *                       The output will still be balanced, i.e. the difference in count between the different
     *                       input elements will at most be 1, but the output will be shuffled in its entirety.
     * @throw std::invalid_argument if cycle_sampling is disabled and `values.size() < num_choices`
     *                              if values is empty and num_choices > 0
     */
    template<typename T>
    Vec<T> choices(const Vec<T>& values, std::size_t num_choices, bool cycle_sampling = false) {
        if (num_choices == 0)
            return {};

        if (values.empty() && num_choices > 0)
            throw std::invalid_argument("Cannot choose from empty values");

        if (values.size() < num_choices) {
            if (!cycle_sampling)
                throw std::invalid_argument("Cannot choose from fewer values than choices");

            auto scrambled = scramble(values);
            auto [num_full_replications, num_remaining] = utils::divmod(num_choices, values.size());

            auto result = Vec<T>::allocated(num_choices);
            for (std::size_t i = 0; i < num_full_replications; ++i) {
                result.extend(scrambled);
            }
            if (num_remaining > 0) {
                result.extend(scrambled.slice(0, num_remaining));
            }

            // Scramble the entire output to ensure that the order of the elements is full random rather than urn-like
            return scramble(result);
        }


        if (num_choices * 5 > values.size()) {
            // If num_choices is close to the size of values, use a shuffle
            return scramble(values).slice(0, num_choices);
        } else {
            // If num_choices is significantly smaller, use reservoir sampling
            auto result = Vec<T>::allocated(num_choices);
            for (std::size_t i = 0; i < num_choices; ++i) {
                result.append(choice(values));
            }
            return result;
        }
    }

    Vec<std::size_t> choice_indices(std::size_t num_values, std::size_t num_choices, bool cycle_sampling = false) {
        return choices(Vec<std::size_t>::range(num_values), num_choices, cycle_sampling);
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
} // namespace serialist


#endif //SERIALISTLOOPER_RANDOM_H
