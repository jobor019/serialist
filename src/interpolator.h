

#ifndef SERIALIST_LOOPER_INTERPOLATOR_H
#define SERIALIST_LOOPER_INTERPOLATOR_H

#include <vector>
#include <optional>
#include <cmath>
#include <sstream>
#include "utils.h"



// ==============================================================================================

class InterpolationStrategy {
public:

    static const char SEPARATOR = ':';

    enum class Type {
        continuation = 1
        , modulo = 2
        , clip = 3
        , pass = 4
    };


    explicit InterpolationStrategy(Type type = Type::clip, float pivot = 1.0)
            : m_type(type), m_pivot(pivot) {}

    static InterpolationStrategy default_strategy() {
        return InterpolationStrategy(Type::clip, 1.0);
    }


    std::string to_string() const {
        std::ostringstream oss;
        oss << static_cast<int>(m_type) << SEPARATOR << m_pivot;
        return oss.str();
    }


    static InterpolationStrategy from_string(const std::string& s) {
        std::istringstream iss(s);

        int type;
        float pivot;
        char dump;

        iss >> type >> dump >> pivot;

        return InterpolationStrategy(static_cast<Type>(type), pivot);
    }


    Type get_type() const { return m_type; }


    float get_pivot() const { return m_pivot; }


private:
    Type m_type;
    float m_pivot;

};


// ==============================================================================================

template<typename T>
class Interpolator {
public:

    static constexpr double epsilon = 1e-6;


    static std::vector<T> interpolate(double position
                                      , const InterpolationStrategy& strategy
                                      , const std::vector<T>& sequence) {
        switch (strategy.get_type()) {
            case InterpolationStrategy::Type::continuation:
                return continuation(position, strategy, sequence);
            case InterpolationStrategy::Type::modulo:
                return modulo(position, strategy, sequence);
            case InterpolationStrategy::Type::clip:
                return clip(position, strategy, sequence);
            case InterpolationStrategy::Type::pass:
                return pass(position, strategy, sequence);
            default:
                throw std::runtime_error("Invalid interpolation type detected");
        }
    }


private:
    static std::vector<T> continuation(double position
                                       , const InterpolationStrategy& strategy
                                       , const std::vector<T>& sequence) {
        if (sequence.empty())
            return {};

        auto index = get_index(utils::modulo(position, 1.0), sequence.size());

        auto element = sequence.at(index);

        if constexpr (utils::is_container<T>::value) {
            std::vector<T> output;
            output.reserve(element.size());
            std::transform(element.begin()
                           , element.end()
                           , output.begin()
                           , [&](T value) { return std::floor(position) * strategy.get_pivot() + value; });
            return output;
        } else {
            return {element};
        }


    }


    static std::vector<T> modulo(double position
                                 , const InterpolationStrategy&
                                 , const std::vector<T>& sequence) {
        if (sequence.empty())
            return {};

        auto index = get_index(utils::modulo(position, 1.0), sequence.size());

        if constexpr (utils::is_container<T>::value) {
            return sequence.at(index);
        } else {
            return {sequence.at(index)};
        }
    }


    static std::vector<T> clip(double position
                               , const InterpolationStrategy&
                               , const std::vector<T>& sequence) {
        if (sequence.empty())
            return {};

        auto index = static_cast<std::size_t>(
                std::max(0l
                         , std::min(static_cast<long>(sequence.size()) - 1
                                    , static_cast<long>(get_index(position, sequence.size()))))
        );

        if constexpr (utils::is_container<T>::value) {
            return sequence.at(index);
        } else {
            return {sequence.at(index)};
        }

    }


    static std::vector<T> pass(double position
                               , const InterpolationStrategy&
                               , const std::vector<T>& sequence) {
        if (sequence.empty() || position < 0)
            return {};

        auto index = get_index(position, sequence.size());

        if (index >= sequence.size())
            return {};

        if constexpr (utils::is_container<T>::value) {
            return sequence.at(index);
        } else {
            return {sequence.at(index)};
        }

    }


    static std::size_t get_index(double position, std::size_t map_size) {
        // This should work correctly up to Mappings of size 67_108_864,
        //   assuming doubles of 8 bytes (tested up to 10_000)
        return static_cast<std::size_t>(
                std::floor(position * static_cast<double>(map_size) + std::copysign(epsilon, position)));
    }
};


#endif //SERIALIST_LOOPER_INTERPOLATOR_H
