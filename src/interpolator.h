

#ifndef SERIALIST_LOOPER_INTERPOLATOR_H
#define SERIALIST_LOOPER_INTERPOLATOR_H

#include <vector>
#include <optional>
#include <cmath>
#include "mapping.h"
#include "utils.h"

template<typename T>
class Interpolator {

public:
    static constexpr double epsilon = 1e-8;

    virtual ~Interpolator() = default;

    Interpolator() = default;
    Interpolator(const Interpolator&) = default;
    Interpolator& operator=(const Interpolator&) = default;
    Interpolator(Interpolator&&) noexcept = default;
    Interpolator& operator=(Interpolator&&) noexcept = default;

    virtual std::vector<T> get(double position, Mapping<T>* mapping) = 0;

protected:
    std::size_t get_index(double position, std::size_t map_size) {
        // This should work correctly up to Mappings of size 67_108_864,
        //   assuming doubles of 8 bytes (tested up to 10_000)
        return static_cast<std::size_t>(
                std::floor(position * static_cast<double>(map_size) + std::copysign(epsilon, position)));
    }

};


// ==============================================================================================

template<typename T>
class ContinueInterpolator : public Interpolator<T> {
public:
    explicit ContinueInterpolator(T focal_point) : m_focal_point(focal_point) {
        static_assert(std::is_arithmetic_v<T>, "T is not a number");
    }


    std::vector<T> get(double position, Mapping<T>* mapping) override {
        if (mapping->empty()) {
            return {};
        }

        auto index = Interpolator<T>::get_index(utils::modulo(position, 1.0), mapping->size());

        auto element = mapping->at(index);

        std::vector<T> output(element.size());
        std::transform(element.begin(), element.end(), output.begin()
                       , [&focal_point = m_focal_point, position](T value) {
                    return std::floor(position) * focal_point + value;
                });

        return output;
    }


    T get_focal_point() const {
        return m_focal_point;
    }


private:
    T m_focal_point;

};


// ==============================================================================================

template<typename T>
class ModuloInterpolator : public Interpolator<T> {
public:

    std::vector<T> get(double position, Mapping<T>* mapping) override {
        if (mapping->empty()) {
            return {};
        }

        auto index = Interpolator<T>::get_index(utils::modulo(position, 1.0), mapping->size());

        return mapping->at(index);
    }
};


// ==============================================================================================

template<typename T>
class ClipInterpolator : public Interpolator<T> {
public:
    std::vector<T> get(double position, Mapping<T>* mapping) override {
        if (mapping->empty()) {
            return {};
        }

        auto index = static_cast<std::size_t>(
                std::max(0l
                         , std::min(static_cast<long>(mapping->size()) - 1
                                    , static_cast<long>(Interpolator<T>::get_index(position, mapping->size())))));

        return mapping->at(index);
    }
};


// ==============================================================================================

template<typename T>
class PassInterpolator : public Interpolator<T> {
public:
    std::vector<T> get(double position, Mapping<T>* mapping) override {
        if (mapping->empty() || position < 0) {
            return {};
        }

        auto i = Interpolator<T>::get_index(position, mapping->size());

        if (i >= mapping->size()) {
            return {};
        }

        return mapping->at(i);
    }

};

#endif //SERIALIST_LOOPER_INTERPOLATOR_H
