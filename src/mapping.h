

#ifndef SERIALIST_LOOPER_MAPPING_H
#define SERIALIST_LOOPER_MAPPING_H

#include <vector>
#include <optional>

#include "interpolator.h"


// TODO: Implement getters, multielement MapElements, etc.

template<typename T>
class MapElement {
public:
    MapElement() = default;

    explicit MapElement(T value) : m_values{value} {}

    MapElement(std::initializer_list<T> values) : m_values{values} {}

    const T& temp_first() const {
        return m_values[0];
    }

    bool has_value() {
        return !m_values.empty();
    }

private:
    std::vector<T> m_values;
};


// ==============================================================================================

template<typename T>
class Mapping {
public:

    Mapping() = default;


    Mapping(std::initializer_list<MapElement<T> > values) : m_mapping{values} {}


    Mapping(std::initializer_list<T> values) : Mapping(std::vector(values)) {}


    explicit Mapping(const std::vector<T>& values) {
        for (auto value: values) {
            m_mapping.emplace_back(MapElement<T>(value));
        }
    }


    std::vector<T> get(std::size_t index) const {
        // TODO: Implement getter with different behaviours for polyphonic MapElements
        if (m_mapping.empty()) {
            return {};
        }

        if (index > m_mapping.size()) {
            index = m_mapping.size() - 1;
        }

        return {m_mapping.at(index).temp_first()};
    }


    [[nodiscard]] std::size_t size() const {
        return m_mapping.size();
    }


    [[nodiscard]] bool empty() const {
        return m_mapping.empty();
    }


    void add(MapElement<T> element, long index = -1) {
        // handle negative indices (insert from end)
        if (index < 0) {
            index += m_mapping.size();
        }

        // clip bounds
        auto position = static_cast<unsigned long>(index) >= m_mapping.size()
                        ? static_cast<long>(m_mapping.size()) : static_cast<long>(index);

        m_mapping.insert(m_mapping.begin() + position, std::move(element));
    }


    void add(std::vector<MapElement<T> > elements, long start_index = -1) {
        if (start_index < 0) {
            start_index += static_cast<long>(m_mapping.size());
        }

        auto position = static_cast<unsigned long>(start_index) >= m_mapping.size()
                        ? static_cast<long>(m_mapping.size()) : static_cast<long>(start_index);

        m_mapping.insert(m_mapping.begin() + position, elements.begin(), elements.end());
    }

    // TODO: Convenience functions for insert, replace, swap, etc. Currently just a dummy
    //  copy of std::vector, but will be relevant later


private:
    std::vector<MapElement<T> > m_mapping;
};


// ==============================================================================================

template<typename T>
class InterpolationMapping {
public:

    InterpolationMapping() = default;


    InterpolationMapping(std::initializer_list<T> values) : InterpolationMapping(std::vector(values)) {}


    explicit InterpolationMapping(
            const std::vector<T>& values
            , std::shared_ptr<Interpolator<T>> interpolator = std::make_shared<ClipInterpolator<T>>()
    )
            : m_mapping(values)
              , m_interpolator(interpolator) {}


    std::optional<T> interpolate(double x) {
        if (!m_mapping.empty() && m_interpolator) {
            return m_interpolator->interpolate(x, m_mapping);
        }
        return std::nullopt;
    }


    [[nodiscard]] std::size_t size() const {
        return m_mapping.size();
    }


    [[nodiscard]] bool empty() const {
        return m_mapping.empty();
    }


    void set_mapping(const std::vector<T>& mapping) {
        m_mapping = mapping;
    }


    void set_interpolator(const std::shared_ptr<Interpolator<T>>& interpolator) {
        m_interpolator = interpolator;
    }


private:
    std::vector<T> m_mapping;
    std::shared_ptr<Interpolator<T>> m_interpolator; // TODO: std::optional rather than std::shared_ptr if possible
};


#endif //SERIALIST_LOOPER_MAPPING_H
