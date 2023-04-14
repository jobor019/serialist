

#ifndef SERIALIST_LOOPER_ACCESSOR_H
#define SERIALIST_LOOPER_ACCESSOR_H

#include <vector>
#include <optional>
#include <cmath>
#include "mapping.h"

template<typename ElementType, typename AccessorType = int>
class Accessor {
public:
    virtual ~Accessor() = default;

    virtual std::vector<ElementType> get(AccessorType position, const Mapping<ElementType>& mapping) = 0;
};


// ==============================================================================================

template<typename T>
class IdentityAccessor : public Accessor<T, int> {
public:

    std::vector<T> get(int position, const Mapping<T>& mapping) override {
        if (position < 0) {
            position += mapping.size();
        }

        if (mapping.empty() || position < 0 || static_cast<std::size_t>(position) >= mapping.size()) {
            return {};
        }

        return mapping.at(static_cast<std::size_t>(position));
    }
};


// ==============================================================================================

template<typename T>
class NthAccessor : public Accessor<T, int> {
public:

    explicit NthAccessor(int nth) : m_nth(nth) {}


    std::vector<T> get(int position, const Mapping<T>& mapping) override {
        if (position < 0) {
            position += mapping.size();
        }

        if (mapping.empty() || position < 0 || static_cast<std::size_t>(position) >= mapping.size()) {
            return {};
        }

        auto element = mapping.at(static_cast<std::size_t>(position));

        // Handle negative indices
        auto sub_element_index = m_nth;
        if (sub_element_index < 0) {
            sub_element_index += element.size();
        }

        if (sub_element_index < 0 || static_cast<std::size_t>(sub_element_index) >= element.size()) {
            return {};
        }

        return {element.at(static_cast<std::size_t>(sub_element_index))};
    }

private:
    int m_nth;
};


// ==============================================================================================

template<typename T>
class FirstAccessor : public NthAccessor<T> {
public:
    FirstAccessor() : NthAccessor<T>(0) {}
};


// ==============================================================================================

template<typename T>
class NthsAccessor : public Accessor<T, int> {
public:
    explicit NthsAccessor(std::vector<int> nths) : m_nths(std::move(nths)) {
        throw std::runtime_error("not implemented yet");
    }

private:
    std::vector<int> m_nths;
};


// ==============================================================================================

template<typename T>
class RandomAccessor : public Accessor<T, int> {
public:
    explicit RandomAccessor() {
        // TODO: Initialize seed
        throw std::runtime_error("not implemented yet");
    }

private:
    // TODO: Seed
};


// ==============================================================================================

template<typename T>
class ContinueInterpolator : public Accessor<T, double> {
public:
    explicit ContinueInterpolator(T focal_point) : m_focal_point(focal_point) {
        static_assert(std::is_arithmetic_v<T>, "T is not a number");
    }

    std::vector<T> get(double position, const Mapping<T>& mapping) override {
        if (mapping.empty()) {
            return {};
        }

        auto index = static_cast<std::size_t>(
                std::floor(static_cast<double>(mapping.size()) * std::fmod(std::fmod(position, 1.0) + 1.0, 1.0)));

        auto element = mapping.at(index);

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
class ModuloInterpolator : public Accessor<T, double> {
public:

    std::vector<T> get(double position, const Mapping<T>& mapping) override {
        if (mapping.empty()) {
            return {};
        }

        auto index = static_cast<std::size_t>(
                std::floor(static_cast<double>(mapping.size()) * std::fmod(std::fmod(position, 1.0) + 1.0, 1.0)));

        return mapping.at(index);
    }
};


// ==============================================================================================

template<typename T>
class ClipInterpolator : public Accessor<T, double> {
public:
    std::vector<T> get(double position, const Mapping<T>& mapping) override {
        if (mapping.empty()) {
            return {};
        }

        auto index = static_cast<std::size_t>(
                std::max(0l
                         , std::min(
                                static_cast<long>(mapping.size()) - 1
                                , static_cast<long>(
                                        std::floor(position * static_cast<double>(mapping.size()))
                                )
                        )
                )
        );

        return mapping.at(index);
    }
};


// ==============================================================================================

template<typename T>
class PassInterpolator : public Accessor<T, double> {
public:
    std::vector<T> get(double position, const Mapping<T>& mapping) override {
        if (mapping.empty() || position < 0) {
            return {};
        }

        auto i = static_cast<std::size_t>(std::floor(position * static_cast<double>(mapping.size())));

        if (i >= mapping.size()) {
            return {};
        }

        return mapping.at(i);
    }

};

#endif //SERIALIST_LOOPER_ACCESSOR_H
