#ifndef SERIALIST_INDEX_H
#define SERIALIST_INDEX_H
#include "core/types/phase.h"


namespace serialist {
// ==============================================================================================

class Index {
public:
    using IndexType = int64_t;

    enum class Strategy { cont, mod, clip, pass };

    explicit Index(IndexType value = 0) : m_value(value) {}

    static Index zero() { return Index{0}; }


    static Index from_index_facet(double index) {
        return Index(static_cast<IndexType>(std::round(index)));
    }


    static Index from_index_facet(const Facet& value) { return from_index_facet(value.get()); }


    static Index from_phase(const Phase& phase, std::size_t map_size) {
        return from_phase_like(phase.get(), map_size);
    }


    static Index from_phase_like(double phase_like, std::size_t map_size) {
        return Index(index_op(phase_like, map_size));
    }


    static IndexType index_op(double position, std::size_t map_size, double epsilon = EPSILON) {
        if (map_size == 0) {
            return 0;
        }

        // edge case: only relevant in Max, where Phase::max() is rounded to Phase::wrap_around(),
        // which will yield incorrect values only in the case of map_size = 1
        if (map_size == 1) {
            return static_cast<IndexType>(position / 1.0);
        }

        // note that we're using 1.0, not Phase::max here. Phase::max is implemented for this exact reason,
        // to ensure that index_op(Phase::max, size) always maps to size - 1
        return static_cast<IndexType>(std::floor(position * static_cast<double>(map_size) + epsilon));
    }

    bool operator==(const Index& other) const { return m_value == other.m_value; }
    bool operator!=(const Index& other) const { return m_value != other.m_value; }
    bool operator<(const Index& other) const { return m_value < other.m_value; }
    bool operator>(const Index& other) const { return m_value > other.m_value; }
    bool operator<=(const Index& other) const { return m_value <= other.m_value; }
    bool operator>=(const Index& other) const { return m_value >= other.m_value; }


    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    bool operator==(const T& other) const { return m_value == other; }


    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    bool operator!=(const T& other) const { return m_value != other; }


    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    bool operator<(const T& other) const { return m_value < other; }


    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    bool operator>(const T& other) const { return m_value > other; }


    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    bool operator<=(const T& other) const { return m_value <= other; }


    template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
    bool operator>=(const T& other) const { return m_value >= other; }


    explicit operator std::string() const { return "Index(" + std::to_string(m_value) + ")"; }


    friend std::ostream& operator<<(std::ostream& os, const Index& index) {
        os << static_cast<std::string>(index) << ")";
        return os;
    }


    IndexType get_raw() const { return m_value; }
    IndexType get_cont() const { return get_raw(); }


    IndexType get_mod(std::size_t size, bool invert = false) const {
        assert(size > 0);
        auto mod_result = m_value % static_cast<IndexType>(size);

        return invert ? apply_inversion(mod_result, size) : mod_result;
    }


    IndexType get_clip(std::size_t size, bool invert = false) const {
        assert(size > 0);

        auto clip_result = utils::clip(m_value, static_cast<IndexType>(0), static_cast<IndexType>(size - 1));
        return invert ? apply_inversion(clip_result, size) : clip_result;
    }


    IndexType get_octave(std::size_t size) const { return m_value / static_cast<IndexType>(size); }


    std::optional<IndexType> get_pass(std::size_t size, bool invert = false) const {
        if (m_value >= 0 && m_value < size)
            return invert ? apply_inversion(m_value, size) : m_value;
        return std::nullopt;
    }


    std::optional<IndexType> get(std::size_t size, Strategy strategy = Strategy::cont, bool invert = false) const {
        switch (strategy) {
            case Strategy::cont:
                return get_cont();
            case Strategy::mod:
                return get_mod(size, invert);
            case Strategy::clip:
                return get_clip(size, invert);
            case Strategy::pass:
                return get_pass(size, invert);
        }
        throw std::runtime_error("Unknown strategy");
    }

private:
    static IndexType apply_inversion(IndexType applied_value, std::size_t size) {
        return static_cast<IndexType>(size) - 1 - applied_value;
    }


    IndexType m_value;
};
}

#endif //SERIALIST_INDEX_H
