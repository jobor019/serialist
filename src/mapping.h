

#ifndef SERIALIST_LOOPER_MAPPING_H
#define SERIALIST_LOOPER_MAPPING_H

#include <vector>
#include <optional>


template <typename T>
using MapElement = std::vector<T>;


// ==============================================================================================

template<typename T>
class Mapping {
public:

    Mapping() = default;


    Mapping(std::initializer_list<MapElement<T> > values) : m_mapping{values} {}


    Mapping(std::initializer_list<T> values) : Mapping(std::vector(values)) {}


    explicit Mapping(const std::vector<T>& values) {
        for (auto value: values) {
            m_mapping.emplace_back(MapElement<T>{value});
        }
    }


    [[nodiscard]] const MapElement<T>& at(std::size_t index) const {
        return m_mapping.at(index);
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
            index += m_mapping.size() + 1;
        }

        // clip bounds
        auto position = static_cast<unsigned long>(index) >= m_mapping.size()
                        ? static_cast<long>(m_mapping.size()) : static_cast<long>(index);

        m_mapping.insert(m_mapping.begin() + position, std::move(element));
    }


    void add(std::vector<MapElement<T> > elements, long start_index = -1) {
        if (start_index < 0) {
            start_index += static_cast<long>(m_mapping.size()) + 1;
        }

        auto position = static_cast<unsigned long>(start_index) >= m_mapping.size()
                        ? static_cast<long>(m_mapping.size()) : static_cast<long>(start_index);

        m_mapping.insert(m_mapping.begin() + position, elements.begin(), elements.end());
    }


    const std::vector<MapElement<T>>& get_values() const {
        return m_mapping;
    }

    // TODO: Convenience functions for insert, replace, swap, etc. Currently just a dummy
    //  copy of std::vector, but will be relevant later


private:
    std::vector<MapElement<T> > m_mapping;
};


// ==============================================================================================

//template<typename DataType>
//class SingleMapping {
//public:
//
//    SingleMapping() = default;
//
//
//    SingleMapping(std::initializer_list<DataType> values) : SingleMapping(std::vector(values)) {}
//
//
//    explicit SingleMapping(
//            const std::vector<DataType>& values
//            , std::shared_ptr<OldInterpolator<DataType>> interpolator = std::make_shared<ClipInterpolation<DataType>>()
//    )
//            : m_mapping(values)
//              , m_interpolator(interpolator) {}
//
//
//    std::optional<DataType> interpolate(double x) {
//        if (!m_mapping.empty() && m_interpolator) {
//            return m_interpolator->interpolate(x, m_mapping);
//        }
//        return std::nullopt;
//    }
//
//
//    [[nodiscard]] std::size_t size() const {
//        return m_mapping.size();
//    }
//
//
//    [[nodiscard]] bool empty() const {
//        return m_mapping.empty();
//    }
//
//
//    void set_mapping(const std::vector<DataType>& mapping) {
//        m_mapping = mapping;
//    }
//
//
//    void set_interpolator(const std::shared_ptr<OldInterpolator<DataType>>& interpolator) {
//        m_interpolator = interpolator;
//    }
//
//
//private:
//    std::vector<DataType> m_mapping;
//    std::shared_ptr<OldInterpolator<DataType>> m_interpolator; // TODO: std::optional rather than std::shared_ptr if possible
//};


#endif //SERIALIST_LOOPER_MAPPING_H
